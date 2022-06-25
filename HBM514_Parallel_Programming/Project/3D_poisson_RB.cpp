#include <mpi.h>
#include <omp.h>
#include <cmath>
#include <iostream>
#include <random>
#include <numeric>
#include <algorithm>
#include <array>
#include <vector>
#include <chrono>
#include <sstream>

#define T float
#define MPI_T MPI_FLOAT
#define MAX_ITER 3
#define X_SIZE 3UL
#define Y_SIZE 3UL
#define Z_SIZE 2UL
#define pos(x,y,z) ((z)*self.nX*self.nY + (y)*self.nX + x)
#define posExt(x,y,z) ((z+1)*(self.nX+2)*(self.nY+2) + (y+1)*(self.nX+2) + (x+1))

struct obj_s {
    int pNum, pW = 1, pH = 1, pD = 1;
    int pId,  pX,     pY,     pZ;    
    MPI_Comm comm3d, commX, commY, commZ;

    std::chrono::system_clock::time_point start, end;
    int iter = 0;

    std::vector<int> pNX, pNY, pNZ;
    std::vector<int> pOffX, pOffY, pOffZ;
    int nX, nY, nZ, matSize;

    std::vector<T> X, Y;

    MPI_Datatype inRW, outRW, inBW, outBW,
                 inRH, outRH, inBH, outBH,
                 inRD, outRD, inBD, outBD;
};

void init(obj_s &self) {
    MPI_Comm_size(MPI_COMM_WORLD, &self.pNum);
    MPI_Comm_rank(MPI_COMM_WORLD, &self.pId);

    for (size_t size = 1; true; size *= 8)
        if (size * 8 <= self.pNum)
            self.pW = self.pH = self.pD *= 2;
        else if (size * 4 <= self.pNum) {
            self.pW = self.pH *= 2; break;
        } else if (size * 2 <= self.pNum) {
            self.pW *= 2; break;
        } else break;

    MPI_Cart_create(MPI_COMM_WORLD, 3, std::vector<int>({self.pW, self.pH, self.pD}).data(), std::vector<int>({0,0,0}).data(), 1, &self.comm3d);
    MPI_Cart_sub(self.comm3d, std::vector<int>({1,0,0}).data(), &self.commX);
    MPI_Cart_sub(self.comm3d, std::vector<int>({0,1,0}).data(), &self.commY);
    MPI_Cart_sub(self.comm3d, std::vector<int>({0,0,1}).data(), &self.commZ);

    MPI_Comm_rank(self.commX, &self.pX);
    MPI_Comm_rank(self.commY, &self.pY);
    MPI_Comm_rank(self.commZ, &self.pZ);

    self.pNX.resize(self.pW, 0); self.pOffX.resize(self.pW, 0);
    for (int i = 1; i < self.pW; ++i) {
        self.pOffX[i] = int(float(X_SIZE) / self.pW * i);
        self.pNX[i-1] = self.pOffX[i] - self.pOffX[i-1];
    }
    self.pNX[self.pW-1] = X_SIZE - self.pOffX[self.pW-1];
    self.nX = self.pNX[self.pX];

    self.pNY.resize(self.pH, 0); self.pOffY.resize(self.pH, 0);
    for (int i = 1; i < self.pH; ++i) {
        self.pOffY[i] = int(float(Y_SIZE) / self.pH * i);
        self.pNY[i-1] = self.pOffY[i] - self.pOffY[i-1];
    }
    self.pNY[self.pH-1] = Y_SIZE - self.pOffY[self.pH-1];
    self.nY = self.pNY[self.pY];

    self.pNZ.resize(self.pD, 0); self.pOffZ.resize(self.pD, 0);
    for (int i = 1; i < self.pD; ++i) {
        self.pOffZ[i] = int(float(Z_SIZE) / self.pD * i);
        self.pNZ[i-1] = self.pOffZ[i] - self.pOffZ[i-1];
    }
    self.pNZ[self.pD-1] = Z_SIZE - self.pOffZ[self.pD-1];
    self.nZ = self.pNZ[self.pZ];

    self.matSize = self.nX*self.nY*self.nZ;

    //self.A = std::vector<T>(self.matSize * self.matSize, 0);
    self.X = std::vector<T>(self.matSize, 0);
    self.Y = std::vector<T>(self.matSize, 0);
    for (size_t x = 0; x < self.nX; ++x)
        for (size_t y = 0; y < self.nY; ++y)
            for (size_t z = 0; z < self.nZ; ++z) {
                self.Y[pos(x,y,z)] = (T)(std::cos(self.pOffX[self.pX]+x) * std::cos(self.pOffY[self.pY]+y) * std::cos(self.pOffZ[self.pZ]+z));
                /*self.A[pos(x,y,z)*self.matSize + pos(x,y,z)] = 1;
                if (x > 0)
                    self.A[pos(x,y,z)*self.matSize + pos(x-1,y,z)] = -1./6;
                if (x < self.nX-1)
                    self.A[pos(x,y,z)*self.matSize + pos(x+1,y,z)] = -1./6;
                if (y > 0)
                    self.A[pos(x,y,z)*self.matSize + pos(x,y-1,z)] = -1./6;
                if (y < self.nY-1)
                    self.A[pos(x,y,z)*self.matSize + pos(x,y+1,z)] = -1./6;
                if (z > 0)
                    self.A[pos(x,y,z)*self.matSize + pos(x,y,z-1)] = -1./6;
                if (z < self.nZ-1)
                    self.A[pos(x,y,z)*self.matSize + pos(x,y,z+1)] = -1./6;*/
            }

    int countR = self.nY*self.nZ/2 + (self.nZ%2)*int(std::ceil(0.5*self.nY));
    int countB = self.nY*self.nZ/2 + (self.nZ%2)*self.nY/2;
    int initPosR = posExt(0,0,0), initPosB = posExt(1,0,0);
    std::vector<int> inOffsR(countR), outOffsR(countR), inOffsB(countB), outOffsB(countB);
    int iR = 0, iB = 0;
    for (int z = 0; z < self.nZ; ++z)
        for (int y = 0; y < self.nY; ++y)
            if ((y+z) % 2 == 0) {
                inOffsR[iR] = pos(0,y,z);
                outOffsR[iR++] = posExt(0,y,z) - initPosR;
            } else {
                inOffsB[iB] = pos(0,y,z);
                outOffsB[iB++] = posExt(0,y,z) - initPosB;
            }
    MPI_Type_indexed(countR, std::vector<int>(countR, 1).data(), inOffsR.data(), MPI_T, &self.inRW);
    MPI_Type_indexed(countR, std::vector<int>(countR, 1).data(), outOffsR.data(), MPI_T, &self.outRW);
    MPI_Type_indexed(countB, std::vector<int>(countB, 1).data(), inOffsB.data(), MPI_T, &self.inBW);
    MPI_Type_indexed(countB, std::vector<int>(countB, 1).data(), outOffsB.data(), MPI_T, &self.outBW);

    iR = iB = 0;
    countR = self.nX*self.nZ/2 + (self.nZ%2)*int(std::ceil(0.5*self.nX));
    countB = self.nX*self.nZ/2 + (self.nZ%2)*self.nX/2;
    inOffsR.resize(countR); outOffsR.resize(countR); inOffsB.resize(countB); outOffsB.resize(countB);
    for (int z = 0; z < self.nZ; ++z)
        for (int x = 0; x < self.nX; ++x)
            if ((x+z) % 2 == 0) {
                inOffsR[iR] = pos(x,0,z);
                outOffsR[iR++] = posExt(x,0,z) - initPosR;
            } else {
                inOffsB[iB] = pos(x,0,z);
                outOffsB[iB++] = posExt(x,0,z) - initPosB;
            }
    MPI_Type_indexed(countR, std::vector<int>(countR, 1).data(), inOffsR.data(), MPI_T, &self.inRH);
    MPI_Type_indexed(countR, std::vector<int>(countR, 1).data(), outOffsR.data(), MPI_T, &self.outRH);
    MPI_Type_indexed(countB, std::vector<int>(countB, 1).data(), inOffsB.data(), MPI_T, &self.inBH);
    MPI_Type_indexed(countB, std::vector<int>(countB, 1).data(), outOffsB.data(), MPI_T, &self.outBH);

    iR = iB = 0;
    countR = self.nX*self.nY/2 + (self.nY%2)*int(std::ceil(0.5*self.nX));
    countB = self.nX*self.nY/2 + (self.nY%2)*self.nX/2;
    inOffsR.resize(countR); outOffsR.resize(countR); inOffsB.resize(countB); outOffsB.resize(countB);
    for (int y = 0; y < self.nY; ++y)
        for (int x = 0; x < self.nX; ++x)
            if ((x+y) % 2 == 0) {
                inOffsR[iR] = pos(x,y,0);
                outOffsR[iR++] = posExt(x,y,0) - initPosR;
            } else {
                inOffsB[iB] = pos(x,y,0);
                outOffsB[iB++] = posExt(x,y,0) - initPosB;
            }
    MPI_Type_indexed(countR, std::vector<int>(countR, 1).data(), inOffsR.data(), MPI_T, &self.inRD);
    MPI_Type_indexed(countR, std::vector<int>(countR, 1).data(), outOffsR.data(), MPI_T, &self.outRD);
    MPI_Type_indexed(countB, std::vector<int>(countB, 1).data(), inOffsB.data(), MPI_T, &self.inBD);
    MPI_Type_indexed(countB, std::vector<int>(countB, 1).data(), outOffsB.data(), MPI_T, &self.outBD);

    MPI_Type_commit(&self.inRW); MPI_Type_commit(&self.outRW); MPI_Type_commit(&self.inBW); MPI_Type_commit(&self.outBW);
    MPI_Type_commit(&self.inRH); MPI_Type_commit(&self.outRH); MPI_Type_commit(&self.inBH); MPI_Type_commit(&self.outBH);
    MPI_Type_commit(&self.inRD); MPI_Type_commit(&self.outRD); MPI_Type_commit(&self.inBD); MPI_Type_commit(&self.outBD);
}

T dot(const obj_s &self, const std::vector<T> &v1, const std::vector<T> &v2, int color)
{
    T v = 0, res;
    // TODO
    for (size_t z = 0; z < self.nZ; ++z)
        for (size_t y = 0; y < self.nY; ++y)
            for (size_t x = (z+y+color)%2; x < self.nX; x += 2)
                v += v1[pos(x,y,z)] * v2[pos(x,y,z)];
    MPI_Allreduce(&v, &res, 1, MPI_T, MPI_SUM, self.comm3d);
    return res;
}

void axpy(const obj_s &self, std::vector<T> &out, T a, const std::vector<T> &X, const std::vector<T> &Y, int color)
{
    for (size_t z = 0; z < self.nZ; ++z)
        for (size_t y = 0; y < self.nY; ++y)
            for (size_t x = (z+y+color)%2; x < self.nX; x += 2)
                out[pos(x,y,z)] = a*X[pos(x,y,z)] + Y[pos(x,y,z)];
}

std::vector<T> matrixVector(const obj_s &self,/*const std::vector<T> &mat,*/ const std::vector<T> &vec, int color) {
    std::vector<T> ret(self.matSize, 0);

    for (size_t z = 0; z < self.nZ; ++z)
        for (size_t y = 0; y < self.nY; ++y)
            for (size_t x = (z+y+color)%2; x < self.nX; x += 2)
                ret[pos(x,y,z)] = vec[posExt(x,y,z)]
                    - vec[posExt(x-1,y,z)]/6 - vec[posExt(x+1,y,z)]/6
                    - vec[posExt(x,y-1,z)]/6 - vec[posExt(x,y+1,z)]/6
                    - vec[posExt(x,y,z-1)]/6 - vec[posExt(x,y,z+1)]/6;
    return ret;
}

void shareVector(const obj_s &self, std::vector<T> &out, const std::vector<T> &in, int color)
{
    MPI_Datatype inW  = color == 0 ? self.inRW : self.inBW,
                 outW = color == 0 ? self.outRW : self.outBW,
                 inH  = color == 0 ? self.inRH : self.inBH,
                 outH = color == 0 ? self.outRH : self.outBH,
                 inD  = color == 0 ? self.inRD : self.inBD,
                 outD = color == 0 ? self.outRD : self.outBD;

    MPI_Status s;
    if (self.pX > 0 && self.pX < self.pW-1) {
        MPI_Sendrecv(&in[pos(0,0,0)],         1, inW, self.pX-1, 0, &out[posExt(self.nX,0,0)], 1, outW, self.pX+1, 0, self.commX, &s);
        MPI_Sendrecv(&in[pos(self.nX-1,0,0)], 1, inW, self.pX+1, 0, &out[posExt(-1,0,0)],      1, outW, self.pX-1, 0, self.commX, &s);
    } else if (self.pX > 0)
        MPI_Sendrecv(&in[pos(0,0,0)],         1, inW, self.pX-1, 0, &out[posExt(-1,0,0)],      1, outW, self.pX-1, 0, self.commX, &s);
    else if (self.pX < self.pW-1)
        MPI_Sendrecv(&in[pos(self.nX-1,0,0)], 1, inW, self.pX+1, 0, &out[posExt(self.nX,0,0)], 1, outW, self.pX+1, 0, self.commX, &s);

    if (self.pY > 0 && self.pY < self.pH-1) {
        MPI_Sendrecv(&in[pos(0,0,0)],         1, inH, self.pY-1, 0, &out[posExt(0,self.nY,0)], 1, outH, self.pY+1, 0, self.commY, &s);
        MPI_Sendrecv(&in[pos(0,self.nY-1,0)], 1, inH, self.pY+1, 0, &out[posExt(0,-1,0)],      1, outH, self.pY-1, 0, self.commY, &s);
    } else if (self.pY > 0)
        MPI_Sendrecv(&in[pos(0,0,0)],         1, inH, self.pY-1, 0, &out[posExt(0,-1,0)],      1, outH, self.pY-1, 0, self.commY, &s);
    else if (self.pY < self.pH-1)
        MPI_Sendrecv(&in[pos(0,self.nY-1,0)], 1, inH, self.pY+1, 0, &out[posExt(0,self.nY,0)], 1, outH, self.pY+1, 0, self.commY, &s);

    if (self.pZ > 0 && self.pZ < self.pD-1) {
        MPI_Sendrecv(&in[pos(0,0,0)],         1, inD, self.pZ-1, 0, &out[posExt(0,0,self.nZ)], 1, outD, self.pZ+1, 0, self.commZ, &s);
        MPI_Sendrecv(&in[pos(0,0,self.nZ-1)], 1, inD, self.pZ+1, 0, &out[posExt(0,0,-1)],      1, outD, self.pZ-1, 0, self.commZ, &s);
    } else if (self.pZ > 0)
        MPI_Sendrecv(&in[pos(0,0,0)],         1, inD, self.pZ-1, 0, &out[posExt(0,0,-1)],      1, outD, self.pZ-1, 0, self.commZ, &s);
    else if (self.pZ < self.pD-1)
        MPI_Sendrecv(&in[pos(0,0,self.nZ-1)], 1, inD, self.pZ+1, 0, &out[posExt(0,0,self.nZ)], 1, outD, self.pZ+1, 0, self.commZ, &s);

    for (size_t z = 0; z < self.nZ; ++z)
        for (size_t y = 0; y < self.nY; ++y)
            for (size_t x = (z+y+color)%2; x < self.nX; x += 2)
                out[posExt(x,y,z)] = in[pos(x,y,z)];


    // std::stringstream ss;
    // ss << self.pId << std::endl;
    // for (int y = 0; y < self.nY; ++y) {
    //     for (int z = 0; z < self.nZ; ++z) {
    //         for (int x = 0; x < self.nX; ++x)
    //             ss << in[pos(x,y,z)] << "\t";
    //         ss << "|\t";
    //     }
    //     ss << "||\t";
    //     for (int z = -1; z < self.nZ+1; ++z)
    //         ss << out[posExt(-1,y,z)] << "\t";
    //     ss << "||\t";
    //     for (int z = -1; z < self.nZ+1; ++z)
    //         ss << out[posExt(self.nX,y,z)] << "\t";
    //     ss << std::endl;
    // }
    // std::cout << ss.str() << std::endl;
        
}

int main(int argc, char** argv)
{
    MPI_Init(&argc, &argv);
    try {
        obj_s self;
        init(self);

        //diff = self.Y;
        std::vector<T> diff = self.Y;
        T dotsDiff[2] = {dot(self, diff, diff, 0), dot(self, diff ,diff, 1)};
        std::vector<T> dir = diff;
        std::vector<T> extendedDir((self.nX+2)*(self.nY+2)*(self.nZ+2), 0);
        shareVector(self, extendedDir, dir, 1);
        for (int z = 0; z < self.nZ; ++z)
            for (int y = 0; y < self.nY; ++y)
                for (int x = (z+y)%2; x < self.nX; x += 2)
                    extendedDir[posExt(x,y,z)] = dir[pos(x,y,z)];

        for (self.iter = 0; true; ++self.iter) {
            for (int color = 0; color < 2; ++color) {
                std::vector<T> Adir = matrixVector(self, /*self.A,*/ extendedDir, color);
                T alpha = dotsDiff[color] / dot(self, dir, Adir, color);
                axpy(self, self.X, alpha, dir, self.X, color); // X = X + alpha * dir

                for (int z = 0; z < self.nZ; ++z)
                    for (int y = 0; y < self.nY; ++y)
                        for (int x = 0; x < self.nX; ++x) {
                            std::cout << self.pOffX[self.pX]+x << "," << self.pOffY[self.pY]+y << "," << self.pOffZ[self.pZ]+z << ": " << Adir[pos(x,y,z)] << std::endl;
                        }
                std::cout << std::endl;

                axpy(self, diff, -alpha, Adir, diff, color); // diff = diff - alpha * A * dir
                if (color == 1) {
                    T lMax[2] = {0,0}, gMax[2];
                    for (size_t i = 0; i < self.matSize; ++i) {
                        lMax[0] = std::max(std::abs(diff[i]), lMax[0]);
                        lMax[1] = std::max(std::abs(self.X[i]), lMax[1]);
                    }
                    MPI_Allreduce(lMax, gMax, 2, MPI_T, MPI_MAX, self.comm3d);
                    if (gMax[0] / gMax[1] < 1e-4 || self.iter >= MAX_ITER-1)
                        goto end_iter;
                }

                T oldDot = dotsDiff[color];
                dotsDiff[color] = dot(self, diff,diff, color);
                T beta = dotsDiff[color] / oldDot;
                axpy(self, dir, beta, dir, diff, color);
                shareVector(self, extendedDir, dir, color);
            }
        }
end_iter:

        std::cout << self.iter+1 << std::endl;
        for (int z = 0; z < self.nZ; ++z)
            for (int y = 0; y < self.nY; ++y)
                for (int x = 0; x < self.nX; ++x) {
                    std::cout << self.pOffX[self.pX]+x << "," << self.pOffY[self.pY]+y << "," << self.pOffZ[self.pZ]+z << ": " << self.X[pos(x,y,z)] << "\t=\t" << self.Y[pos(x,y,z)] << std::endl;
                }
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        MPI_Finalize();
        return 1;
    }
    MPI_Finalize();
    return 0;
}