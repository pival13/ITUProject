#pragma once

#ifndef RAND_SEED
#   define RAND_SEED 1653933600 // 2022-04-30T18:00:00Z //time(nullptr)
#endif

#ifndef MAT_SIZE
#   define MAT_SIZE 50000UL
#endif

#include <mpi.h>
#include <cmath>
#include <iostream>
#include <random>
#include <numeric>
#include <algorithm>
#include <vector>
#include <chrono>

struct obj_s {
    int pNum, pId;    
    MPI_Comm comm2d, commCol, commRow;
    int pW = 1, pH = 1, pX, pY;

    std::chrono::system_clock::time_point start, end4, end7;
    int iter = 0, iter4 = 0;

    int width, height;
    std::vector<float> A, B, X;
};
