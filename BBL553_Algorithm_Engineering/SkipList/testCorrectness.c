#include <stdio.h>
#include <time.h>
#include <string.h>

#include "SkipList.h"

#include "testData.h"

int comp(const int *a, const int *b)
{
    return *a - *b;
}

void dump(const int *v)
{
    printf("%d", *v);
}

int main()
{
    srand(RANDOM_SEED);

    printf("Creating SkipList\n");
    SkipList *list = new_SkipList((comparator)comp);

    for (size_t i = 0; i < numbersSize; ++i) {
        printf("Inserting %d: ", numbersSample[i]);
        SkipList_insert(list, (int*)numbersSample+i);
        SkipList_dump(list, (dumper)dump);
    }
    SkipList_dumpLayer(list, (dumper)dump);
    printf("\n");

    char tmpBuff[256] = {0};
    for (size_t i = 0; i < searchSize; ++i) {
        snprintf(tmpBuff, 255, " (%s)", numberToSeach[i].comment ? numberToSeach[i].comment : "");
        printf("Is there %d?%s ", numberToSeach[i].nb, numberToSeach[i].comment ? tmpBuff : "");
        printf("%s\n", SkipList_search(list, &numberToSeach[i].nb) != NULL ? "True" : "False");
    }
    printf("\n");

    for (size_t i = 0; i < removeSize; ++i) {
        snprintf(tmpBuff, 255, " (%s)", numberToRemove[i].comment ? numberToRemove[i].comment : "");
        printf("Removing %d%s: ", numberToRemove[i].nb, numberToRemove[i].comment ? tmpBuff : "");
        void *res = SkipList_remove(list, &numberToRemove[i].nb);
        if (res != NULL) {
            SkipList_dump(list, (dumper)dump);
        } else
            printf("Failed.\n");
    }

    printf("Deleting list\n");
    delete_SkipList(list);
}