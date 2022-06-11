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

static void deleteBranch(SLNode *node)
{
    while (node != NULL) {
        for (uint8_t i = 0; i < node->height-1; ++i)
            deleteBranch(node->nexts[i]);
        SLNode *old = node;
        node = node->nexts[node->height-1];
        free(old);
    }
}
void delete_SkipList(SkipList *list)
{
    for (uint8_t i = 0; i < list->head->height; ++i)
        deleteBranch(list->head->nexts[i]);
    free(list->head);
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

    // Insert node
    for (uint8_t height = head->height; height >= node->height; --height)
        while (head->nexts[height-1] != NULL && list->cmp(value, head->nexts[height-1]->value) >= 0)
            head = head->nexts[height-1];
    node->nexts[node->height-1] = head->nexts[node->height-1];
    head->nexts[node->height-1] = node;

    // Move lower nodes
    for (uint8_t height = node->height-2; height != (uint8_t)-1; --height) {
        while (head->nexts[height] != NULL && list->cmp(value, head->nexts[height]->value) >= 0)
            head = head->nexts[height];
        node->nexts[height] = head->nexts[height];
        head->nexts[height] = NULL;
    }
}

void *SkipList_remove(SkipList *list, const void *value)
{
    SLNode *head = list->head;
    SLNode *node;

    // Detach node
    for (uint8_t height = head->height-1; height != (uint8_t)-1; --height) {
        if (head->nexts[height] == NULL || list->cmp(value, head->nexts[height]->value) < 0) continue;
        while (head->nexts[height]->nexts[height] != NULL && list->cmp(value, head->nexts[height]->nexts[height]->value) >= 0)
            head = head->nexts[height];
        if (list->cmp(value, head->nexts[height]->value) == 0) {
            node = head->nexts[height];
            head->nexts[height] = node->nexts[height];
            goto SkipList_remove_nodeFound;
        }
        head = head->nexts[height];
    }
    // Failed to find the node
    return NULL;

SkipList_remove_nodeFound:
    // Reattach lower nodes
    for (uint8_t height = node->height-2; height != (uint8_t)-1; --height) {
        while (head->nexts[height] != NULL)
            head = head->nexts[height];
        head->nexts[height] = node->nexts[height];
    }

    while (list->head->height > 0 && list->head->nexts[list->head->height-1] == NULL)
        --list->head->height;

    void *data = node->value;
    free(node);
    return data;
}

static size_t sizeBranch(const SLNode *branch)
{
    size_t size = 0;
    while (branch != NULL) {
        ++size;
        for (uint8_t i = 0; i < branch->height-1; ++i)
            size += sizeBranch(branch->nexts[i]);
        branch = branch->nexts[branch->height-1];
    }
    return size;
}
size_t SkipList_size(const SkipList *list) {
    size_t size = 0;
    for (uint8_t i = 0; i < list->head->height; ++i)
        size += sizeBranch(list->head->nexts[i]);
    return size;
}

static void dump(const SLNode *node, dumper f)
{
    while (node != NULL) {
        f(node->value);
        printf(",\t");
        for (uint8_t i = 0; i < node->height-1; ++i)
            dump(node->nexts[i], f);
        node = node->nexts[node->height-1];
    }
}
void SkipList_dump(const SkipList *list, dumper f) {
    for (uint8_t i = 0; i < list->head->height; ++i)
        dump(list->head->nexts[i], f);
    printf("\n");
}

static void dumpBranchLevel(const SLNode *head, uint8_t height, dumper f)
{
    while (head != NULL) {
        if (height == head->height-1) {
            f(head->value);
            printf(",\t");
            for (uint8_t h = 0; h < height; ++h)
                for (size_t padding = sizeBranch(head->nexts[h]); padding != 0; --padding)
                    printf("\t");
        } else {
            if (head->nexts[height] != NULL)
                printf("|\t");
            else
                printf("\t");
            for (uint8_t h = 0; h < height; ++h)
                for (size_t padding = sizeBranch(head->nexts[h]); padding != 0; --padding)
                    printf("\t");
            dumpBranchLevel(head->nexts[height], height, f);
        }
        head = head->nexts[head->height-1];
    }
}
void SkipList_dumpLayer(const SkipList *list, dumper f) {
    for (uint8_t height = list->head->height-1; height != (uint8_t)-1; --height) {
        if (height == list->head->height-1)
            printf("*\t");
        else if (list->head->nexts[height] != NULL)
            printf("|\t");
        else
            printf("\t");
        for (uint8_t h = 0; h < height; ++h)
            for (size_t padding = sizeBranch(list->head->nexts[h]); padding != 0; --padding)
                printf("\t");
        dumpBranchLevel(list->head->nexts[height], height, f);
        for (uint8_t h = height+1; h < list->head->height; ++h)
            dumpBranchLevel(list->head->nexts[h], height, f);
        printf("\n");
    }
}