#include <stdint.h>
#include <stdlib.h>

typedef struct _SkipList SkipList;

typedef int (*comparator)(const void*,const void*);
typedef void (*dumper)(const void*);

SkipList *new_SkipList(comparator keyComparator);
void delete_SkipList(SkipList *list);

void SkipList_insert(SkipList *list, void *value);
void *SkipList_remove(SkipList *list, const void *value);
void *SkipList_search(const SkipList *list, const void *value);

size_t SkipList_size(const SkipList *list);

void SkipList_dump(const SkipList *list, dumper);
void SkipList_dumpLayer(const SkipList *list, dumper);