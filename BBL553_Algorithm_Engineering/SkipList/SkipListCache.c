#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <malloc.h>

#ifdef _WIN32
#include <intrin.h>
#else
#include <x86intrin.h>
#endif

#include "SkipList.h"

#ifndef ELEM_PER_NODE
#define ELEM_PER_NODE 2
#endif

typedef struct SkipListNode {
    int *value;
    struct SkipListNode **nexts;
    struct SkipListNode *prev;
    const uint8_t height; // Max = 32
    const uint8_t internalIndex;
} SLNode;
static SLNode *newNodes(uint8_t height);

struct _SkipList {
    comparator cmp;
    SLNode *head;
};


SkipList *new_SkipList(comparator valueComparator)
{
    SkipList *list = (SkipList*)malloc(sizeof(SkipList));
    list->cmp = valueComparator;
    
#if _DEBUG
    list->head = (SLNode*)malloc(sizeof(SLNode));
    memcpy(list->head, &(SLNode){
        .value = NULL, .height = 0, .prev = NULL,
        .internalIndex = ELEM_PER_NODE - 1,
        .nexts = NULL
    }, sizeof(SLNode));
#else
    void *mem = malloc(sizeof(SLNode) + 32*sizeof(void*));
    list->head = (SLNode*)mem;
    memcpy(list->head, &(SLNode){
        .value = NULL, .height = 0, .prev = NULL,
        .internalIndex = ELEM_PER_NODE - 1,
        .nexts = memset((uint8_t*)mem + sizeof(SLNode), 0, 32*sizeof(void*))
    }, sizeof(SLNode));
#endif
    return list;
}

static void deleteBranch(SLNode *node)
{
    while (node != NULL) {
        for (uint8_t i = 0; i < node->height-1; ++i)
            deleteBranch(node->nexts[i]);
        SLNode *old = node;
        node = node->nexts[node->height-1];
        if (old->internalIndex == ELEM_PER_NODE-1 || (old+1)->prev == NULL)
            free(old - old->internalIndex);
    }
}
void delete_SkipList(SkipList *list)
{
    for (uint8_t i = 0; i < list->head->height; ++i)
        deleteBranch(list->head->nexts[i]);
    free(list->head);
    free(list);
}

static SLNode *search(const SkipList *list, const void *value)
{
    SLNode *head = list->head;
    for (uint8_t height = head->height; height > 0; --height)
        while (head->nexts[height-1] != NULL && list->cmp(value, head->nexts[height-1]->value) >= 0)
            head = head->nexts[height-1];
    // No value
    if (head == list->head || list->cmp(value, head->value) != 0)
        return NULL;
    return head;
}
void *SkipList_search(const SkipList *list, const void *value) { const SLNode *node = search(list, value); return node ? node->value : NULL; }

static uint8_t randomHeight();
static SLNode *moveNode(SLNode *dest, const SLNode *src);
static void shiftNodesRight(SLNode *node);
void SkipList_insert(SkipList *list, void *value)
{
    SLNode *head = list->head;
    const uint8_t height = randomHeight();
    if (height > head->height)
        memcpy((void*)(&head->height), &height, sizeof(uint8_t));

    // Get the previous node
    for (uint32_t level = head->height; level >= height; --level)
        while (head->nexts[level-1] != NULL && list->cmp(value, head->nexts[level-1]->value) >= 0)
            head = head->nexts[level-1];

    SLNode *node;
    // On the same block than previous node
    if (head->height == height && head->internalIndex < ELEM_PER_NODE-1) {
        // Block is full
        if ((head - head->internalIndex + ELEM_PER_NODE-1)->prev)
            moveNode(newNodes(height), head - head->internalIndex + ELEM_PER_NODE-1);
        node = head + 1;
        shiftNodesRight(head + 1);
    // Next block available with free space
    } else if (head->nexts[height-1] != NULL && !(head->nexts[height-1] - head->nexts[height-1]->internalIndex + ELEM_PER_NODE-1)->prev) {
        node = head->nexts[height-1];
        shiftNodesRight(head->nexts[height-1]);
    } else
        node = newNodes(height);

    // Insert node
    node->value = value;
    node->nexts[height-1] = head->nexts[height-1];
    head->nexts[height-1] = node;
    node->prev = head;
    if (node->nexts[height-1] != NULL)
        node->nexts[height-1]->prev = node;
    
    // Link lower heights
    for (uint32_t level = height-1; level > 0; --level) {
        while (head->nexts[level-1] != NULL && list->cmp(value, head->nexts[level-1]->value) >= 0)
            head = head->nexts[level-1];
        node->nexts[level-1] = head->nexts[level-1];
        head->nexts[level-1] = NULL;
        if (node->nexts[level-1])
            node->nexts[level-1]->prev = node;
    }
}

static void shiftNodesLeft(SLNode *node);
void *SkipList_remove(SkipList *list, const void *value)
{
    SLNode *node = search(list, value);
    if (!node) return NULL;

    SLNode *prev = node->prev;
    prev->nexts[node->height-1] = NULL;
    for (uint8_t height = node->height; height > 0; --height) {
        while (prev->nexts[height-1] != NULL) prev = prev->nexts[height-1];
        prev->nexts[height-1] = node->nexts[height-1];
        if (prev->nexts[height-1])
            prev->nexts[height-1]->prev = prev;
    }

    void *data = node->value;
    shiftNodesLeft(node);
    // Block is empty
    if ((node - node->internalIndex)->prev == NULL)
        free(node - node->internalIndex);
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
        printf("[%d:%d", node->height, node->internalIndex);
        if (node->internalIndex != ELEM_PER_NODE-1 && (node+1)->prev == NULL)
            printf(":%d", ELEM_PER_NODE-node->internalIndex-1);
        printf("],\t");
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

void SkipList_dumpLayer(const SkipList *list, dumper f)
{
    SkipList_dump(list, f);
}

static SLNode *newNodes(uint8_t height)
{
    void *mem = (SLNode*)malloc(ELEM_PER_NODE * (sizeof(SLNode) + height*sizeof(void*)));
    memset((uint8_t*)mem + ELEM_PER_NODE*sizeof(SLNode), 0, ELEM_PER_NODE * height * sizeof(void*));

    for (uint8_t idx = 0; idx < ELEM_PER_NODE; ++idx)
        memcpy((SLNode*)mem + idx, &(SLNode){
            .value = NULL,
            .internalIndex = idx,
            .height = height,
            .nexts = (SLNode**)((uint8_t*)mem + ELEM_PER_NODE*sizeof(SLNode) + idx*height*sizeof(void*)),
            .prev = NULL
        }, sizeof(SLNode));
    return (SLNode*)mem;
}

static uint8_t randomHeight()
{
    uint32_t v = rand() << 1;
#ifdef _WIN32
    return _tzcnt_u32(v);
#else
    return __builtin_ctz(v);
#endif
}

static SLNode *moveNode(SLNode *dest, const SLNode *src)
{
    dest->value = src->value;
    dest->prev = src->prev;
    memcpy(dest->nexts, src->nexts, src->height*sizeof(void*));
    if (dest->prev)
        dest->prev->nexts[dest->height-1] = dest;
    for (uint8_t height = 0; height < dest->height; ++height)
        if (dest->nexts[height])
            dest->nexts[height]->prev = dest;
    return dest;
}

static void shiftNodesRight(SLNode *node)
{
    SLNode *block = node - node->internalIndex;
    for (uint8_t idx = ELEM_PER_NODE-1; idx > node->internalIndex; --idx)
        moveNode(block + idx, block + idx - 1);
}

static void shiftNodesLeft(SLNode *node)
{
    SLNode *block = node - node->internalIndex;
    for (uint8_t idx = node->internalIndex; idx < ELEM_PER_NODE-1; ++idx)
        moveNode(block + idx, block + idx + 1);
    moveNode(block + ELEM_PER_NODE-1, &(SLNode){
        .value = NULL,
        .height = node->height,
        .internalIndex = ELEM_PER_NODE-1,
        .prev = NULL,
        .nexts = memset(alloca(node->height*sizeof(void*)), 0, node->height*sizeof(void*))
    });
}