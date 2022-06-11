#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <malloc.h>

#include "SkipList.h"

typedef struct SkipListNode {
    void *value;
    struct SkipListNode **nexts;
    uint32_t levels;
} SLNode;


struct _SkipList {
    comparator cmp;
    SLNode *head;
};

static SLNode *newNode(void *value, uint32_t level)
{
    SLNode *node = (SLNode*)malloc(sizeof(SLNode));
    node->value = value;
    node->levels = level;
    node->nexts = calloc(level, sizeof(SLNode*));
    return node;
}

SkipList *new_SkipList(comparator valueComparator)
{
    SkipList *list = (SkipList*)malloc(sizeof(SkipList));
    list->cmp = valueComparator;
    list->head = newNode(NULL, 1);
    return list;
}

void delete_SkipList(SkipList *list)
{
    SLNode *node = list->head;
    while (node != NULL) {
        SLNode *old = node;
        node = old->nexts[0];
        free(old->nexts);
        free(old);
    }
    free(list);
}

void *SkipList_search(const SkipList *list, const void *value)
{
    const SLNode *head = list->head;
    for (uint32_t level = head->levels; level > 0; --level)
        while (head->nexts[level-1] != NULL && list->cmp(value, head->nexts[level-1]->value) > 0) 
            head = head->nexts[level-1];
    head = head->nexts[0];
    if (head == NULL || list->cmp(value, head->value) != 0) // No value
        return NULL;
    return head->value;
}

static uint32_t randomLevel()
{
    uint32_t i = 1;
    while (rand() % 2 == 1) ++i;
    return i;
}

void SkipList_insert(SkipList *list, void *value)
{
    SLNode *node = newNode(value, randomLevel());
    SLNode *head = list->head;

    if (node->levels > head->levels) {
        head->nexts = realloc(head->nexts, node->levels*sizeof(SLNode*));
        for (uint32_t level = head->levels; level < node->levels; ++level)
            head->nexts[level] = NULL;
        head->levels = node->levels;
    }

    for (uint32_t level = head->levels; level > 0; --level) {
        while (head->nexts[level-1] != NULL && list->cmp(value, head->nexts[level-1]->value) >= 0)
            head = head->nexts[level-1];
        if (level <= node->levels) {
            node->nexts[level-1] = head->nexts[level-1];
            head->nexts[level-1] = node;
        }
    }
}

void *SkipList_remove(SkipList *list, const void *value)
{
    SLNode *head = list->head;
    SLNode **prevNodes = alloca(sizeof(SLNode*) * head->levels);

    for (uint32_t level = head->levels; level > 0; --level) {
        while (head->nexts[level-1] != NULL && list->cmp(value, head->nexts[level-1]->value) > 0) 
            head = head->nexts[level-1];
        prevNodes[level-1] = head;
    }
    head = head->nexts[0];
    if (head == NULL || list->cmp(value, head->value) != 0) // No value
        return NULL;
    for (uint32_t level = head->levels; level > 0; --level)
        prevNodes[level-1]->nexts[level-1] = head->nexts[level-1];
    void *data = head->value;
    free(head->nexts);
    free(head);
    return data;
}

size_t SkipList_size(const SkipList *list)
{
    size_t size = 0;
    const SLNode *head = list->head;
    while (head->nexts[0] != NULL) {
        head = head->nexts[0];
        ++size;
    }
    return size;
}

void SkipList_dump(const SkipList *list, dumper f)
{
    const SLNode *head = list->head;
    while (head->nexts[0] != NULL) {
        f(head->nexts[0]->value);
        printf(",\t");
        head = head->nexts[0];
    }
    printf("\n");
}

void SkipList_dumpLayer(const SkipList *list, dumper f)
{
    for (uint32_t level = list->head->levels; level > 0; --level) {
        const SLNode *head = list->head;
        while (head->nexts[0] != NULL) {
            if (head->nexts[0]->levels >= level) {
                f(head->nexts[0]->value);
                printf(",\t");
            } else
                printf("\t");
            head = head->nexts[0];
        }
        printf("\n");
    }
}
