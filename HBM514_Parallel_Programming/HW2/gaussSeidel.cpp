#include "LinearEquation.h"

#ifndef MAX_ITER
#   define MAX_ITER MAT_SIZE
#endif

static void split(obj_s &self, std::vector<int> offs, std::vector<int> counts)
{
    std::vector<float> B = std::move(self.B);
    self.B.resize(self.height);
    MPI_Scatterv(B.data(), counts.data(), offs.data(), MPI_FLOAT, self.B.data(), MAT_SIZE, MPI_FLOAT, 0, self.commCol);

    std::vector<float> A = std::move(self.A);
    self.A.resize(self.height * self.width);
    for (int i = 0; i < self.pH; ++i) {
        offs[i] *= self.width;
        counts[i] *= self.width;
    }
    MPI_Scatterv(A.data(), counts.data(), offs.data(), MPI_FLOAT, self.A.data(), self.height * self.width, MPI_FLOAT, 0, self.commCol);
    MPI_Barrier(self.commRow);
}

void gaussSeidel(obj_s &self)
{
    // counts[i] = number of elements on process i
    // offs[i] = index of first element of process i
    std::vector<int> offs(self.pNum, 0), counts(self.pNum);
    for (int i = 1; i < self.pNum; ++i) {
        offs[i] = int(float(MAT_SIZE) / self.pNum * i);
        counts[i-1] = offs[i] - offs[i-1];
    }
    counts[self.pNum-1] = MAT_SIZE - offs[self.pNum-1];

    self.pH = self.pNum;
    self.pY = self.pId;
    self.height = counts[self.pY];
    self.commCol = MPI_COMM_WORLD;
    self.pW = 1;
    self.pX = 0;
    self.width = MAT_SIZE;
    self.commRow = MPI_COMM_SELF;
    int initI = offs[self.pY];

    split(self, offs, counts);

    self.X = std::vector<float>(self.width, 0.f);
    for (self.iter = 1; self.iter <= MAX_ITER; ++self.iter) {
        std::vector<float> oldX = std::move(self.X);
        std::vector<float> newX(self.height, 0.f);
        self.X.resize(self.width);
        // Calculate even X
        for (int i = initI%2; i < self.height; i += 2) {
            float sum = 0;
            for (int j = 0; j < initI + i; ++j)
                sum += self.A[i*self.width + j] * oldX[j];
            for (int j = initI+i+1; j < self.width; ++j)
                sum += self.A[i*self.width + j] * oldX[j];
            newX[i] = (self.B[i] - sum) / self.A[i*self.width + initI+i];
        }
        MPI_Allgatherv(newX.data(), self.height, MPI_FLOAT, self.X.data(), counts.data(), offs.data(), MPI_FLOAT, self.commCol);

        // Calculate odd X
        for (int i = (initI+1)%2; i < self.height; i += 2) {
            float sum = 0;
            for (int j = 0; j < initI + i; j += 2) // Even
                sum += self.A[i*self.width + j] * self.X[j];
            for (int j = 1; j < initI + i; j += 2) // Odd <i
                sum += self.A[i*self.width + j] * oldX[j];
            for (int j = initI+i+2; j < self.width; j += 2) // Odd >i
                sum += self.A[i*self.width + j] * oldX[j];
            newX[i] = (self.B[i] - sum) / self.A[i*self.width + (initI+i)];
        }
        MPI_Allgatherv(newX.data(), self.height, MPI_FLOAT, self.X.data(), counts.data(), offs.data(), MPI_FLOAT, self.commCol);

        // Find the highest diff and X
        float maxDiff = 0, maxX = 0;
        for (int i = 0; i < self.width; ++i) {
            maxDiff = std::max(std::abs(self.X[i] - oldX[i]), maxDiff);
            maxX = std::max(std::abs(self.X[i]), maxX);
        }

        if (self.pId == 0 && maxDiff/maxX < 1e-4 && self.iter4 == 0) {
            self.end4 = std::chrono::system_clock::now();
            self.iter4 = self.iter;
        }
        if (maxDiff/maxX < 1e-7)
            break;
    }
}