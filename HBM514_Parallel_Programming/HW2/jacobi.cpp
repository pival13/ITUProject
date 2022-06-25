#include "LinearEquation.h"

#ifndef MAX_ITER
#   define MAX_ITER MAT_SIZE
#endif

static void split(obj_s &self, std::vector<int> offs, std::vector<int> counts)
{
    std::vector<float> B = std::move(self.B);
    self.B.resize(self.width);
    MPI_Scatterv(B.data(), counts.data(), offs.data(), MPI_FLOAT, self.B.data(), self.width, MPI_FLOAT, 0, self.commRow);

    MPI_Datatype colsS, colsL;
    MPI_Type_vector(MAT_SIZE, MAT_SIZE/self.pW, MAT_SIZE, MPI_FLOAT, &colsS);
    MPI_Type_vector(MAT_SIZE, MAT_SIZE/self.pW + 1, MAT_SIZE, MPI_FLOAT, &colsL);
    MPI_Type_commit(&colsS), MPI_Type_commit(&colsL);
    std::vector<float> A = std::move(self.A);
    self.A.resize(self.height * self.width);
    if (self.pX == 0) {
        MPI_Request r;
        for (int i = 0; i < self.pW; ++i)
            if (counts[i] == counts[0])
                MPI_Isend(&A[offs[i]], 1, colsS, i, 0, self.commRow, &r);
            else
                MPI_Isend(&A[offs[i]], 1, colsL, i, 0, self.commRow, &r);
    }
    MPI_Status s;
    MPI_Recv(self.A.data(), self.height*self.width, MPI_FLOAT, 0, 0, self.commRow, &s);
    MPI_Barrier(self.commRow);
    MPI_Type_free(&colsS), MPI_Type_free(&colsL);
}

void jacobi(obj_s &self)
{
    // counts[i] = number of elements on process i
    // offs[i] = index of first element of process i
    std::vector<int> offs(self.pNum, 0), counts(self.pNum);
    for (int i = 1; i < self.pNum; ++i) {
        offs[i] = int(float(MAT_SIZE) / self.pNum * i);
        counts[i-1] = offs[i] - offs[i-1];
    }
    counts[self.pNum-1] = MAT_SIZE - offs[self.pNum-1];

    self.pH = 1;
    self.pY = 0;
    self.height = MAT_SIZE;
    self.commCol = MPI_COMM_SELF;
    self.pW = self.pNum;
    self.pX = self.pId;
    self.width = counts[self.pX];
    self.commRow = MPI_COMM_WORLD;

    split(self, offs, counts);

    self.X.resize(self.width, 0.f);
    for (self.iter = 1; self.iter <= MAX_ITER; ++self.iter) {
        // Calculate for all i sum(A[i,j] * x[j]), with limited j
        std::vector<float> sums(self.height);
        for (int i = 0; i < self.height; ++i) {
            for (int j = 0; j < self.width; ++j)
                if (i != offs[self.pX] + j)
                    sums[i] += self.A[i*self.width + j] * self.X[j];
        }
        // Send the sum to the process calculating x[i] and retrieve its owns
        // Received format: [P0_SumX0,P0_SumX1,...,P0_SumXN,P1_SumX0,...]
        std::vector<float> eqs(self.width * self.pW);
        std::vector<int> off(self.pW); for (int i = 0; i < self.pW; ++i) off[i] = i*self.width;
        MPI_Alltoallv(sums.data(), counts.data(), offs.data(), MPI_FLOAT, eqs.data(), std::vector<int>(self.pW, self.width).data(), off.data(), MPI_FLOAT, self.commRow);
        // Calculate x[i] = (B[i] - sum(P*_SumXi)) / A[i,i]
        std::vector<float> oldX = std::move(self.X);
        self.X.resize(self.width, 0.f);
        for (int i = 0; i < self.width; ++i) {
            for (int p = 0; p < self.pW; ++p)
                self.X[i] += eqs[p*self.width + i];
            self.X[i] = (self.B[i] - self.X[i]) / self.A[(offs[self.pX]+i)*self.width + i];
        }

        // Find the highest diff and X
        float lMax[2] = {0,0}, gMax[2];
        for (int i = 0; i < self.width; ++i) {
            lMax[0] = std::max(std::abs(self.X[i] - oldX[i]), lMax[0]);
            lMax[1] = std::max(std::abs(self.X[i]), lMax[1]);
        }
        MPI_Allreduce(&lMax, &gMax, 2, MPI_FLOAT, MPI_MAX, self.commRow);

        if (self.pId == 0 && gMax[0]/gMax[1] < 1e-4 && self.iter4 == 0) {
            self.end4 = std::chrono::system_clock::now();
            self.iter4 = self.iter;
        }
        if (gMax[0]/gMax[1] < 1e-7)
            break;
    }

    std::vector<float> X = std::move(self.X);
    if (self.pX == 0) self.X.resize(MAT_SIZE);
    MPI_Gatherv(X.data(), self.width, MPI_FLOAT, self.X.data(), counts.data(), offs.data(), MPI_FLOAT, 0, self.commRow);
}