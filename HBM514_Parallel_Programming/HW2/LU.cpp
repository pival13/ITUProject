#include "LinearEquation.h"

static void split(obj_s &self)
{
    MPI_Request r; MPI_Status s;
    std::vector<float> A = std::move(self.A);
    std::vector<float> B = std::move(self.B);
    if (self.pX == 0) {
        self.B.resize(MAT_SIZE/self.pH);
        // Send B to the first of every row
        MPI_Scatter(B.data(), MAT_SIZE/self.pH, MPI_FLOAT, self.B.data(), MAT_SIZE/self.pH, MPI_FLOAT, 0, self.commCol);
        B = std::move(self.B);
        // Forward B to the diagonal element of the row
        if (self.pW == self.pH)
            MPI_Isend(B.data(), self.width, MPI_FLOAT, self.pY, 0, self.commRow, &r);
        else {
            MPI_Isend(B.data(), self.width, MPI_FLOAT, self.pY*2, 0, self.commRow, &r);
            MPI_Isend(&B[self.width], self.width, MPI_FLOAT, self.pY*2+1, 0, self.commRow, &r);
        }
    }
    int initI = self.height * self.pY;
    int initJ = self.width * self.pX;
    if (initI <= initJ && initJ < initI + self.height) {
        self.B.resize(self.width);
        MPI_Recv(self.B.data(), self.width, MPI_FLOAT, 0, 0, self.commRow, &s);
    }

    // Split A ammong the rows
    if (self.pX == 0) {
        self.A.resize(self.height * MAT_SIZE);
        MPI_Scatter(A.data(), self.height*MAT_SIZE, MPI_FLOAT, self.A.data(), self.height*MAT_SIZE, MPI_FLOAT, 0, self.commCol);
        A = std::move(self.A);
    }
    // Split A among the columns
    MPI_Datatype subMat;
    MPI_Type_vector(self.height, self.width, MAT_SIZE, MPI_FLOAT, &subMat);
    MPI_Type_commit(&subMat);
    self.A.resize(self.height * self.width);
    if (self.pX == 0)
        for (int i = 0; i < self.pW; ++i)
            MPI_Isend(A.data()+self.width*i, 1, subMat, i, 0, self.commRow, &r);
    MPI_Recv(self.A.data(), self.height*self.width, MPI_FLOAT, 0, 0, self.commRow, &s);

    MPI_Barrier(self.comm2d);
    MPI_Type_free(&subMat);
}

static void gather(obj_s &self)
{
    MPI_Request r; MPI_Status s;
    int initI = self.height * self.pY;
    int initJ = self.width * self.pX;
    // Send X to the first element of the row
    if (initI <= initJ && initJ < initI + self.height)
        MPI_Isend(self.X.data(), self.width, MPI_FLOAT, 0, 0, self.commRow, &r);

    if (self.pX == 0) {
        std::vector<float> X(MAT_SIZE / self.pH);
        if (self.pW == self.pH)
            MPI_Recv(X.data(), self.width, MPI_FLOAT, self.pY, 0, self.commRow, &s);
        else {
            MPI_Recv(X.data(), self.width, MPI_FLOAT, self.pY*2, 0, self.commRow, &s);
            MPI_Recv(&X[self.width], self.width, MPI_FLOAT, self.pY*2+1, 0, self.commRow, &s);
        }
        if (self.pY == 0)
            self.X.resize(MAT_SIZE);
        // Gather X from the first column
        MPI_Gather(X.data(), MAT_SIZE/self.pH, MPI_FLOAT, self.X.data(), MAT_SIZE/self.pH, MPI_FLOAT, 0, self.commCol);
    }
}

static void factorization(obj_s &self)
{
    int initI = self.height * self.pY;
    int initJ = self.width * self.pX;

    for (int k = 0; k < MAT_SIZE-1; ++k) {
        std::vector<float> left(self.height), up(self.width);
        if (k < initI && k < initJ) {
            // XY > k => Lower right part
            MPI_Bcast(up.data(), self.width, MPI_FLOAT, k / self.height, self.commCol);
            MPI_Bcast(left.data(), self.height, MPI_FLOAT, k / self.width, self.commRow);
            for (int i = std::max(0, k-initI); i < self.height; ++i)
                for (int j = std::max(0, k-initJ); j < self.width; ++j)
                    self.A[i*self.width + j] -= left[i] * up[j];
        } else if (k < initI+self.height && k < initJ+self.width) {
            if (k >= initI && k >= initJ) {
                // XY+1 > k => XY => Middle part
                for (int i = k-initI+1; i < self.height; ++i) {
                    self.A[i*self.width + k-initJ] /= self.A[(k-initI)*self.width + k-initJ];
                    for (int j = k-initJ+1; j < self.width; ++j)
                        self.A[i*self.width + j] -= self.A[i*self.width + k-initJ] * self.A[(k-initI)*self.width + j];
                }
                if (self.pH != 1)
                    MPI_Bcast(&self.A[(k-initI)*self.width + k-initJ], self.width-(k-initJ), MPI_FLOAT, self.pY, self.commCol);
                if (self.pW != 1 && k-initI+1 != self.height) {
                    for (int i = k-initI+1; i < self.height; ++i)
                        left[i] = self.A[i*self.width + k-initJ];
                    MPI_Bcast(&left[k-initI+1], self.height-(k-initI+1), MPI_FLOAT, self.pX, self.commRow);
                }
            } else if (k >= initI) {
                // X+1 > k, Y >= k > Y+1 => Middle right part
                if (k-initI+1 != self.height)
                    MPI_Bcast(&left[k-initI+1], self.height-(k-initI+1), MPI_FLOAT, k / self.width, self.commRow);
                for (int i = k-initI+1; i < self.height; ++i)
                    for (int j = 0; j < self.width; ++j)
                        self.A[i*self.width + j] -= left[i] * self.A[(k-initI)*self.width + j];
                if (self.pH != 1)
                    MPI_Bcast(&self.A[(k-initI)*self.width], self.width, MPI_FLOAT, self.pY, self.commCol);
            } else {
                // X+1 > k >= X, Y+1 > k => Middle lower part
                MPI_Bcast(&up[k-initJ], self.width-(k-initJ), MPI_FLOAT, k / self.height, self.commCol);
                for (int i = 0; i < self.height; ++i) {
                    self.A[i*self.width + k-initJ] /= up[k-initJ];
                    for (int j = k-initJ+1; j < self.width; ++j)
                        self.A[i*self.width + j] -= self.A[i*self.width + k-initJ] * up[j];
                }
                if (self.pW != 1) {
                    for (int i = 0; i < self.height; ++i)
                        left[i] = self.A[i*self.width + k-initJ];
                    MPI_Bcast(left.data(), self.height, MPI_FLOAT, self.pX, self.commRow);
                }
            }
        } else if (k < initI+self.height-1) {
            if (k >= initI)
                // X < k, Y+1 > k >= Y => Middle left part
                MPI_Bcast(&left[k-initI+1], self.height-(k-initI+1), MPI_FLOAT, k / self.width, self.commRow);
            else
                // X < k, Y > k => Bottom left part
                MPI_Bcast(left.data(), self.height, MPI_FLOAT, k / self.width, self.commRow);
        } else if (k < initJ+self.width && self.pH != 1) {
            if (k >= initJ)
                // X+1 > k >= X, Y < k => Middle upper part
                MPI_Bcast(&up[k-initJ], self.width-(k-initJ), MPI_FLOAT, k / self.height, self.commCol);
            else
                // X > k, Y < k => Upper right part
                MPI_Bcast(up.data(), self.width, MPI_FLOAT, k / self.height, self.commCol);
        }
        //MPI_Barrier(self.comm2d);
    }
}

void LU(obj_s &self)
{
    for (int i = 0; self.pW * self.pH < self.pNum; ++i)
        if (i % 2 == 0) self.pW *= 2;
        else            self.pH *= 2;
    MPI_Cart_create(MPI_COMM_WORLD, 2, std::vector<int>({self.pW, self.pH}).data(), std::vector<int>({0,0}).data(), true, &self.comm2d);
    MPI_Cart_sub(self.comm2d, std::vector<int>({1,0}).data(), &self.commRow);
    MPI_Cart_sub(self.comm2d, std::vector<int>({0,1}).data(), &self.commCol);
    MPI_Comm_rank(self.commRow, &self.pX);
    MPI_Comm_rank(self.commCol, &self.pY);

    self.width = MAT_SIZE / self.pW;
    self.height = MAT_SIZE / self.pH;

    split(self);

    factorization(self);

    self.X.resize(self.width);
    int initI = self.height * self.pY;
    int initJ = self.width * self.pX;

    // left[i] = sum(A[i,j] * X[j])
    // up[i] = X[i]
    if (initJ <= initI + self.height - 1) {
        MPI_Status s; MPI_Request r;
        std::vector<float> left(self.height, 0.f), up(self.width, 0.f);
        if (self.pX > 0)
            MPI_Recv(left.data(), self.height, MPI_FLOAT, self.pX-1, 0, self.commRow, &s);
        if (initJ <= initI - 1)
            MPI_Recv(up.data(), self.width, MPI_FLOAT, self.pY-1, 0, self.commCol, &s);
        int initI2 = std::max(0, self.pX*self.width-initI);
        for (int i = initI2; i < self.height; ++i) {
            for (int j = 0; j < std::min(initI + i - initJ + 1, self.width); ++j) {
                if (initI + i == initJ + j)
                    self.X[j] = up[j] = self.B[j] - left[i];
                else
                    left[i] += self.A[i*self.width + j] * up[j];
            }
        }
        // up is send below, to each process
        if (self.pY+1 != self.pH)
            MPI_Isend(up.data(), self.width, MPI_FLOAT, self.pY+1, 0, self.commCol, &r);
        // left send to the right, up to the diagonal
        if (initJ + self.width <= initI + self.height - 1)
            MPI_Isend(left.data(), self.height, MPI_FLOAT, self.pX+1, 0, self.commRow, &r);
    }

    // right[i] = sum(A[i,j] * X[j])
    // down[i] = X[i]
    if (initJ + self.width - 1 >= initI) {
        MPI_Status s; MPI_Request r;
        std::vector<float> right(self.height, 0.f), down(self.width, 0.f);
        if (self.pX+1 < self.pW)
            MPI_Recv(right.data(), self.height, MPI_FLOAT, self.pX+1, 0, self.commRow, &s);
        if (initJ + self.width - 1 >= initI + self.height)
            MPI_Recv(down.data(), self.width, MPI_FLOAT, self.pY+1, 0, self.commCol, &s);
        int initI2 = std::min(self.height-1, ((self.pX+1)*self.width-1-initI));
        for (int i = initI2; i >= 0; --i) {
            for (int j = self.width-1; j >= std::max(0, initI + i - initJ); --j) {
                if (initI + i == initJ + j)
                    self.X[j] = down[j] = (self.X[j] - right[i]) / self.A[i*self.width + j];
                else
                    right[i] += self.A[i*self.width + j] * down[j];
            }
        }
        // down is send above, to each process
        if (self.pY > 0)
            MPI_Isend(down.data(), self.width, MPI_FLOAT, self.pY-1, 0, self.commCol, &r);
        // right is send to the left, until the diagonal
        if (initJ - 1 >= initI)
            MPI_Isend(right.data(), self.height, MPI_FLOAT, self.pX-1, 0, self.commRow, &r);
    }

    gather(self);
}
