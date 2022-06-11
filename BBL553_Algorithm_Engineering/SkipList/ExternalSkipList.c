// (C) 2013 by Troy Deck; https://github.com/tdeck/c-skiplist
#include "SkipList.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>

// This defines the maximum number of next pointers that can be stored in a
// single node. For good performance, a good rule of thumb is to set this to
// lg(N), where:
//      lg() is the base-2 logarithm function
//      N is the maximum number of entries in your list
//  If you set this to 1, you will get a slightly slower sorted linked list
//  implementation.
#define MAX_SKIPLIST_HEIGHT     8

struct _SkipList {
    void * value;
    comparator cmp;
    int height;
    SkipList * next[MAX_SKIPLIST_HEIGHT];
};


void sl_free_entry(SkipList * entry);

// Returns a random number in the range [1, max] following the geometric
// distribution.
static int grand (int max) {
    int result = 1;

    while (result < max && (rand() > RAND_MAX / 2)) {
        ++ result;
    }

    return result;
}

// Returns a sentinel node representing the head node of a new skip list.
SkipList * new_SkipList(comparator cmp) {
    // Construct and return the head sentinel
    SkipList * head = calloc(1, sizeof(SkipList)); // Calloc will zero out next
    if (!head) return NULL; // Out-of-memory check
    head->height = MAX_SKIPLIST_HEIGHT;
    head->cmp = cmp;
    return head;
}

// Frees all nodes in the skiplist
void delete_SkipList(SkipList * head) {
    SkipList * current_entry = head;
    SkipList * next_entry = NULL;
    while (current_entry) {
        next_entry = current_entry->next[0];
        sl_free_entry(current_entry);
        current_entry = next_entry;
    }
}

// Searches for an entry in the skip list, and returns
// the associated value, or NULL if it was not found.
void * SkipList_search(const SkipList * head, const void * value) {
    const SkipList * curr = head;
    int level = head->height - 1;

    // Find the position where the key is expected
    while (curr != NULL && level >= 0) {
        if (curr->next[level] == NULL) {
            -- level;
        } else {
            int cmp = head->cmp(curr->next[level]->value, value);
            if (cmp == 0) { // Found a match
                return curr->next[level]->value;
            } else if (cmp > 0) { // Drop down a level
                -- level;
            } else { // Keep going at this level
                curr = curr->next[level];
            }
        }
    }
    // Didn't find it
    return NULL;
}

// Inserts a value into the skip list, replacing it if
// it is already in the list.
void SkipList_insert(SkipList * head, void * value) {
    SkipList * prev[MAX_SKIPLIST_HEIGHT];
    SkipList * curr = head;
    int level = head->height - 1;

    // Find the position where the key is expected
    while (curr != NULL && level >= 0) {
        prev[level] = curr;
        if (curr->next[level] == NULL) {
            -- level;
        } else {
            int cmp = head->cmp(curr->next[level]->value, value);
            if (cmp == 0) { // Found a match, replace the old value
                curr->next[level]->value = value;
                return;
            } else if (cmp > 0) { // Drop down a level
                -- level;
            } else { // Keep going at this level
                curr = curr->next[level];
            }
        }
    }

    // Didn't find it, we need to insert a new entry
    SkipList * new_entry = malloc(sizeof(SkipList));
    new_entry->height = grand(head->height);
    new_entry->value = value;
    int i;
    // Null out pointers above height
    for (i = MAX_SKIPLIST_HEIGHT - 1; i > new_entry->height; -- i) {
        new_entry->next[i] = NULL;
    }
    // Tie in other pointers
    for (i = new_entry->height - 1; i >= 0; -- i) {
        new_entry->next[i] = prev[i]->next[i];
        prev[i]->next[i] = new_entry;
    }
}

// Frees the memory allocated for a skiplist entry.
void sl_free_entry(SkipList * entry) {
    entry->value = NULL;

    free(entry);
    entry = NULL;
}

// Removes a key, value association from the skip list.
void * SkipList_remove(SkipList * head, const void * value) {
    SkipList * prev[MAX_SKIPLIST_HEIGHT];
    SkipList * curr = head;
    int level = head->height - 1;

    // Find the list node just before the condemned node at every
    // level of the chain
    int cmp = 1;
    while (curr != NULL && level >= 0) {
        prev[level] = curr;
        if (curr->next[level] == NULL) {
            -- level;
        } else {
            cmp = head->cmp(curr->next[level]->value, value);
            if (cmp >= 0) { // Drop down a level
                -- level;
            } else { // Keep going at this level
                curr = curr->next[level];
            }
        }
    }

    // We found the match we want, and it's in the next pointer
    if (curr && !cmp) {
        SkipList * condemned = curr->next[0];
        void * value = condemned->value;
        // Remove the condemned node from the chain
        int i;
        for (i = condemned->height - 1; i >= 0; -- i) {
          prev[i]->next[i] = condemned->next[i];
        }
        // Free it
        sl_free_entry(condemned);
        condemned = NULL;
        return value;
    }
    return NULL;
}

size_t SkipList_size(const SkipList *head) {
    size_t size = 0;
    const SkipList * curr = head->next[0];

    // Iterate over every item of the list
    while (curr != NULL) {
        ++ size;
        curr = curr->next[0];
    }
    return size;
}

void SkipList_dump(const SkipList * head, dumper dump) {
    const SkipList * curr = head->next[0];

    // Iterate over every item of the list
    while (curr != NULL) {
        dump(curr->value);
        printf(", ");
        curr = curr->next[0];
    }
    printf("\n");
}

void SkipList_dumpLayer(const SkipList * head, dumper dump) { 
    for (int level = head->height - 1; level >= 0; --level) {
        const SkipList * curr = head->next[level];

        printf("%d: ", level + 1);
        while (curr != NULL) {
            dump(curr->value);
            printf(", ");
            curr = curr->next[level];
        }
        printf("\n");
    }
}