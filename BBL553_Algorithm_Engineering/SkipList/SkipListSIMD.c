#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <malloc.h>

#include "SkipList.h"

#if _WIN32
#include <intrin.h>
#define tzcnt _tzcnt_u32
#else
#include <x86intrin.h>
#define tzcnt __builtin_ctz
#endif

typedef struct SkipListNode {
    void *value;
    struct SkipListNode **nexts;
    uint8_t height; // Max = 32
} SLNode;

struct _SkipList {
    comparator cmp;
    SLNode *head;
};

static SLNode *newNode(void *value, uint8_t height)
{
    SLNode *node = (SLNode*)malloc(sizeof(SLNode) + height*sizeof(SLNode*));
    memcpy(node, &(SLNode){
        .value = value,
        .height = height,
        .nexts = memset(node+1, 0, height*sizeof(SLNode*)),
    }, sizeof(SLNode));
    return node;
}

SkipList *new_SkipList(comparator valueComparator)
{
    SkipList *list = (SkipList*)malloc(sizeof(SkipList));
    list->cmp = valueComparator;
    // 32 is the maximum height (number of bits on int32_t)
    list->head = newNode(NULL, 32);
    list->head->height = 0;
    return list;
}

void delete_SkipList(SkipList *list)
{
    SLNode *node = list->head;
    while (node != NULL) {
        SLNode *old = node;
        node = old->nexts[0];
        free(old);
    }
    free(list);
}

static SLNode *lookup(const SkipList *list, const void *value, SLNode **prevs, uint8_t stopBefore);
void *SkipList_search(const SkipList *list, const void *value)
{
    const SLNode *node = lookup(list, value, alloca(list->head->height * sizeof(SLNode*)), 0);
    return node != list->head && list->cmp(value, node->value) == 0 ? node->value : NULL;
}

static uint8_t randomLevel()
{
    // tzcnt is define at the beginning
    return (uint8_t)tzcnt(rand() << 1);
}

void SkipList_insert(SkipList *list, void *value)
{
    SLNode *node = newNode(value, randomLevel());
    if (node->height > list->head->height)
        list->head->height = node->height;
    SLNode **prevs = alloca(list->head->height * sizeof(SLNode*));
    SLNode *prev = lookup(list, value, prevs, 0);

    uint8_t height = node->height-1;
    for (; height > prev->height-1; --height) {
        node->nexts[height] = prevs[height]->nexts[height];
        prevs[height]->nexts[height] = node;
    }
    for (; height != (uint8_t)-1; --height) {
        node->nexts[height] = prev->nexts[height];
        prev->nexts[height] = node;
    }
}

void *SkipList_remove(SkipList *list, const void *value)
{
    SLNode **prevs = alloca(list->head->height * sizeof(SLNode*));
    SLNode *prev = lookup(list, value, prevs, 1);
    SLNode *node = prev->nexts[0];

    // No value
    if (node == NULL || list->cmp(value, node->value) != 0)
        return NULL;
    // Move to the last element which compare true
    while (node->nexts[0] != NULL && list->cmp(value, node->nexts[0]->value) == 0) {
        for (uint8_t height = prev->height-1; height > node->height-1; --height)
            prevs[height] = prev;
        prev = node;
        node = node->nexts[0];
    }

    uint8_t height = node->height-1;
    for (; height > prev->height-1; --height)
        prevs[height]->nexts[height] = node->nexts[height];
    for (; height != (uint8_t)-1; --height)
        prev->nexts[height] = node->nexts[height];

    void *data = node->value;
    free(node);
    return data;
}

size_t SkipList_size(const SkipList *list)
{
    size_t size = 0;
    for (SLNode *head = list->head; (head = head->nexts[0]) != NULL; ++size);
    return size;
}

void SkipList_dump(const SkipList *list, dumper f)
{
    const SLNode *head = list->head->nexts[0];
    while (head != NULL) {
        f(head->value);
        printf(",\t");
        head = head->nexts[0];
    }
    printf("\n");
}

void SkipList_dumpLayer(const SkipList *list, dumper f)
{
    for (uint8_t height = list->head->height-1; height != (uint8_t)-1; --height) {
        const SLNode *head = list->head;
        while (head->nexts[0] != NULL) {
            if (head->nexts[0]->height > height) {
                f(head->nexts[0]->value);
                printf(",\t");
            } else
                printf("\t");
            head = head->nexts[0];
        }
        printf("\n");
    }
}

/*
         2      0
  +------+------+
   \      \ 
  3 +      + 1
*/
static SLNode *lookup(const SkipList *list, const void *value, SLNode **prevs, uint8_t stopBefore)
{
    SLNode *head = list->head;
    uint8_t height = head->height-1;
    for (uint8_t i = 0; i < head->height; ++i)
        prevs[i] = head;
    // -2 instead of -1 because of cases 4/8
    while (height < (uint8_t)-2) {
        int32_t compared[4] = {-INT32_MAX,-INT32_MAX,-INT32_MAX,-INT32_MAX};
        if (head->nexts[height] != NULL) {
            // Node forward
            compared[2] = list->cmp(value, head->nexts[height]->value);
            if (head->nexts[height]->nexts[height] != NULL)
                // Node forward-forward
                compared[0] = list->cmp(value, head->nexts[height]->nexts[height]->value);
            if (height > 0) {
                // Node below
                compared[3] = list->cmp(value, head->nexts[height-1]->value);
                if (head->nexts[height]->nexts[height-1] != NULL)
                    // Node forward-below
                    compared[1] = list->cmp(value, head->nexts[height]->nexts[height-1]->value);
            }
        } else if (height > 0 && head->nexts[height-1] != NULL)
            // Node below
            compared[3] = list->cmp(value, head->nexts[height-1]->value);
    
        __m128i zeros = _mm_set1_epi32(0);
        int match;
        if (stopBefore) {
            // Get last node < value
            // res[] = value > node ? -1 : 0
            __m128i res = _mm_cmpgt_epi32(*(__m128i*)compared, zeros);
            match = _mm_cmpestri(_mm_set1_epi16(-1), 1, res, 7, _SIDD_SWORD_OPS | _SIDD_CMP_EQUAL_ANY | _SIDD_LEAST_SIGNIFICANT);
        } else {
            // Get last node <= value
            // res[] = value < node ? -1 : 0
            __m128i res = _mm_cmplt_epi32(*(__m128i*)compared, zeros);
            match = _mm_cmpestri(zeros, 1, res, 7, _SIDD_SWORD_OPS | _SIDD_CMP_EQUAL_ANY | _SIDD_LEAST_SIGNIFICANT);
        }
        switch (match) {
        case 0:
            prevs[height] = head->nexts[height];
            head = head->nexts[height]->nexts[height]; break;
        case 2:
            prevs[height] = head->nexts[height];
            head = head->nexts[height]->nexts[height-1]; --height; break;
        case 6:
            prevs[height] = head;
            head = head->nexts[height-1]; --height; break;
        case 4:
            prevs[height] = head;
            head = head->nexts[height]; // Fall down
        case 8:
            height -= 2;
            break;
        }
    }
    return head;
}