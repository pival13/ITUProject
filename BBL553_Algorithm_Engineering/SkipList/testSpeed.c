#include <stdio.h>
#include <time.h>

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

void runTest(SkipList *list, int *numbers)
{
    int64_t t0, t1;
    size_t size;
    int failed = 0;

    printf("Insertion: ");
    t0 = clock();
    for (size_t i = 0; i < NUMBER_COUNT; ++i)
        SkipList_insert(list, numbers+i);
    t1 = clock();
    size = SkipList_size(list);
    printf("%lld. List size: %llu\nSearch: ", t1-t0, size);
    t0 = clock();
    for (size_t i = 0; i < NUMBER_COUNT; ++i)
        SkipList_search(list, numbers+i);
    t1 = clock();
    printf("%lld\nDeletion: ", t1-t0);
    t0 = clock();
    //static int a = 0;
    //++a;
    for (size_t i = 0; i < NUMBER_COUNT; ++i) {
        if (SkipList_remove(list, numbers+i) == NULL)
            ++failed;
        //if (i % (NUMBER_COUNT / 10) == 0 && a == 3)
        //    SkipList_dumpLayer(list, dump);
    }
    t1 = clock();
    size = SkipList_size(list);
    printf("%lld. %d failed. List size: %llu.\n\n", t1-t0, failed, size);
}

int main()
{
    srand(RANDOM_SEED);

    printf("Testing %d elements\n", NUMBER_COUNT);
    SkipList *list = new_SkipList((comparator)comp);
#if _DEBUG && NUMBER_COUNT <= 10000
    int ints[NUMBER_COUNT];
#else
    int *ints = malloc(sizeof(int)*NUMBER_COUNT);
#endif

    printf("Test with ordered elements\n");
    for (size_t i = 0; i < NUMBER_COUNT; ++i)
        ints[i] = (int)i;
    //runTest(list, ints);

    printf("Test with reversed ordered elements\n");
    for (size_t i = 0; i < NUMBER_COUNT; ++i)
        ints[i] = (int)(NUMBER_COUNT-i);
    //runTest(list, ints);
    //delete_SkipList(list);

    //list = new_SkipList((comparator)comp);
    printf("Test with random elements\n");
    for (size_t i = 0; i < NUMBER_COUNT; ++i)
        ints[i] = (int)rand();
    runTest(list, ints);

    delete_SkipList(list);
#if !_DEBUG || NUMBER_COUNT > 10000
    free(ints);
#endif
}