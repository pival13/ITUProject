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
#if _DEBUG
    int *value;
    struct SkipListNode *nexts[32];
#else
    void *value;
    struct SkipListNode **nexts;
#endif
    uint8_t height; // Max = 32
} SLNode;

struct _SkipList {
    comparator cmp;
    SLNode *head;
};

static SLNode *newNode(void *value, uint8_t height)
{
#if _DEBUG
    SLNode *node = (SLNode*)malloc(sizeof(SLNode));
    memcpy(node, &(SLNode){
        .value = value,
        .height = height,
        .nexts = NULL,
    }, sizeof(SLNode));
#else
    SLNode *node = (SLNode*)malloc(sizeof(SLNode) + height*sizeof(SLNode*));
    memcpy(node, &(SLNode){
        .value = value,
        .height = height,
        .nexts = memset(node+1, 0, height*sizeof(SLNode*)),
    }, sizeof(SLNode));
#endif
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

void *SkipList_search(const SkipList *list, const void *value)
{
    const SLNode *head = list->head;
    for (uint8_t height = head->height-1; height != (uint8_t)-1; --height)
        while (head->nexts[height] != NULL && list->cmp(value, head->nexts[height]->value) >= 0) 
            head = head->nexts[height];
    return head != list->head && list->cmp(value, head->value) == 0 ? head->value : NULL;
}

static uint8_t randomLevel()
{
    // tzcnt is define at the beginning
    return (uint8_t)tzcnt(rand() << 1);
}

void SkipList_insert(SkipList *list, void *value)
{
    SLNode *node = newNode(value, randomLevel());
    SLNode *head = list->head;
    
    if (node->height > head->height)
        head->height = node->height;

    for (uint8_t height = head->height-1; height != (uint8_t)-1; --height) {
        while (head->nexts[height] != NULL && list->cmp(value, head->nexts[height]->value) >= 0)
            head = head->nexts[height];
        if (height < node->height) {
            node->nexts[height] = head->nexts[height];
            head->nexts[height] = node;
        }
    }
}

void *SkipList_remove(SkipList *list, const void *value)
{
#if _DEBUG
    SLNode *prevs[32];
#else
    SLNode **prevs = alloca(list->head->height * sizeof(SLNode*));
#endif
    SLNode *head = list->head;

    for (uint8_t height = head->height-1; height != (uint8_t)-1; --height) {
        while (head->nexts[height] != NULL && list->cmp(value, head->nexts[height]->value) > 0)
            head = head->nexts[height];
        prevs[height] = head;
    }
    head = head->nexts[0];

    if (head == NULL || list->cmp(value, head->value) != 0) // No value
        return NULL;
    for (uint8_t height = head->height-1; height != (uint8_t)-1; --height)
        prevs[height]->nexts[height] = head->nexts[height];
    void *data = head->value;
    free(head);
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
