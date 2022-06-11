#pragma once

#include <stdint.h>

#ifndef RANDOM_SEED
#define RANDOM_SEED 20221202 // Date of the beginning of the exam: 2022-12-02
#endif
#ifndef NUMBER_COUNT
#define NUMBER_COUNT 1000000 // 1,000,000
#endif


typedef struct {int nb;const char*comment;} search_t;

static const size_t numbersSize = 7;
static const int numbersSample[] = {1,20,-6,99,50,1,0,5,1,-4,20,5000,6,-3,19,0,1,23,-7,30};

static const size_t searchSize = 6;
static const search_t numberToSeach[] = {
    {1,     NULL},
    {-6,    "Min"},
    {99,    "Max"},
    {2,     "Absent"},
    {-10,   "Absent < min"},
    {100,   "Absent > max"},
};

static const size_t removeSize = 7;
static const search_t numberToRemove[] = {
    {20,    NULL},
    {1,     "Duplicate"},
    {-6,    "Min"},
    {99,    "Max"},
    {5,     "Absent"},
    {-6,    "Absent < min"},
    {10000, "Absent > max"},
};
