#include <mpi.h>
//#include <omp.h>
#include <cmath>
#include <iostream>
#include <vector>
#include <chrono>

#define T float
#define MPI_T MPI_FLOAT
#define MAX_ITER 10000
#define X_SIZE 250UL
#define Y_SIZE 250UL
#define Z_SIZE 250UL
#define pos(x,y,z) ((z)*self.nX*self.nY + (y)*self.nX + x)
#define posExt(x,y,z) ((z+1)*(self.nX+2)*(self.nY+2) + (y+1)*(self.nX+2) + (x+1))

struct obj_s {
    int pNum, pW = 1, pH = 1, pD = 1;
    int pId,  pX,     pY,     pZ;    
    MPI_Comm comm3d, commX, commY, commZ;

    std::chrono::system_clock::time_point start_all, start, end, end_all;
    int iter = 0;

    std::vector<int> pNX, pNY, pNZ;
    std::vector<int> pOffX, pOffY, pOffZ;
    int nX, nY, nZ, matSize;

    std::vector<T> X, Y, diff, dir;
    T dotDiff;

    MPI_Datatype  inW,  inH,  inD,
                 outW, outH, outD;
};

void init(obj_s &self) {
    MPI_Comm_size(MPI_COMM_WORLD, &self.pNum);
    MPI_Comm_rank(MPI_COMM_WORLD, &self.pId);

    // Distribute processes on X, Y and Z
    for (size_t size = 1; true; size *= 8)
        if (size * 8 <= self.pNum)
            self.pW = self.pH = self.pD *= 2;
        else if (size * 4 <= self.pNum) {
            self.pW = self.pH *= 2; break;
        } else if (size * 2 <= self.pNum) {
            self.pW *= 2; break;
        } else break;


    // Create topology
    MPI_Cart_create(MPI_COMM_WORLD, 3, std::vector<int>({self.pW, self.pH, self.pD}).data(), std::vector<int>({0,0,0}).data(), 1, &self.comm3d);
    MPI_Cart_sub(self.comm3d, std::vector<int>({1,0,0}).data(), &self.commX);
    MPI_Cart_sub(self.comm3d, std::vector<int>({0,1,0}).data(), &self.commY);
    MPI_Cart_sub(self.comm3d, std::vector<int>({0,0,1}).data(), &self.commZ);

    MPI_Comm_rank(self.commX, &self.pX);
    MPI_Comm_rank(self.commY, &self.pY);
    MPI_Comm_rank(self.commZ, &self.pZ);


    // Determine the range of X,Y and Z each processes handles
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


    // Fill the Y vector with our f function
    self.X = std::vector<T>(self.matSize, 0);
    self.Y = std::vector<T>(self.matSize, 0);
    for (size_t x = 0; x < self.nX; ++x)
        for (size_t y = 0; y < self.nY; ++y)
            for (size_t z = 0; z < self.nZ; ++z)
                self.Y[pos(x,y,z)] = (T)(std::cos(self.pOffX[self.pX]+x) * std::cos(self.pOffY[self.pY]+y) * std::cos(self.pOffZ[self.pZ]+z));

    
    // Create MPI datatype for ghost points exchange
    int i = 0;
    int initPos = posExt(0,0,0);
    std::vector<int> inOffs(self.nY*self.nZ), outOffs(self.nY*self.nZ), cnts(self.nY*self.nZ, 1);
    for (int z = 0; z < self.nZ; ++z)
        for (int y = 0; y < self.nY; ++y, ++i) {
            inOffs[i] = pos(0,y,z);
            outOffs[i] = posExt(0,y,z) - initPos;
        }
    MPI_Type_indexed(self.nY*self.nZ, cnts.data(), inOffs.data(), MPI_T, &self.inW);
    MPI_Type_indexed(self.nY*self.nZ, cnts.data(), outOffs.data(), MPI_T, &self.outW);
    
    i = 0;
    inOffs.resize(self.nX*self.nZ); outOffs.resize(self.nX*self.nZ); cnts.resize(self.nX*self.nZ, 1);
    for (int z = 0; z < self.nZ; ++z)
        for (int x = 0; x < self.nX; ++x, ++i) {
            inOffs[i] = pos(x,0,z);
            outOffs[i] = posExt(x,0,z) - initPos;
        }
    MPI_Type_indexed(self.nX*self.nZ, cnts.data(), inOffs.data(), MPI_T, &self.inH);
    MPI_Type_indexed(self.nX*self.nZ, cnts.data(), outOffs.data(), MPI_T, &self.outH);
    
    i = 0;
    inOffs.resize(self.nX*self.nY); outOffs.resize(self.nX*self.nY); cnts.resize(self.nX*self.nY, 1);
    for (int y = 0; y < self.nY; ++y)
        for (int x = 0; x < self.nX; ++x, ++i) {
            inOffs[i] = pos(x,y,0);
            outOffs[i] = posExt(x,y,0) - initPos;
        }
    MPI_Type_indexed(self.nX*self.nY, cnts.data(), inOffs.data(), MPI_T, &self.inD);
    MPI_Type_indexed(self.nX*self.nY, cnts.data(), outOffs.data(), MPI_T, &self.outD);

    MPI_Type_commit(&self.inW); MPI_Type_commit(&self.outW);
    MPI_Type_commit(&self.inH); MPI_Type_commit(&self.outH);
    MPI_Type_commit(&self.inD); MPI_Type_commit(&self.outD);
}

void gatherX(obj_s &self, std::vector<int> &offsX, std::vector<int> &offsY, std::vector<int> &offsZ, std::vector<int> &offsP)
{
    std::vector<T> X;
    std::vector<int> cnts;
    // Gather all X to front processes
    if (self.pZ == 0) {
        X.resize(self.nX*self.nY*Z_SIZE, 0);
        offsZ.resize(self.pD, 0); cnts.resize(self.pD, 0);
        cnts[0] = self.nX*self.nY*self.nZ;
        for (int i = 1; i < self.pD; ++i) {
            cnts[i] = self.nX*self.nY*self.pNZ[i];
            offsZ[i] = offsZ[i-1] + cnts[i-1];
        }
    }
    MPI_Gatherv(self.X.data(), self.nX*self.nY*self.nZ, MPI_T, X.data(), cnts.data(), offsZ.data(), MPI_T, 0, self.commZ);

    // Gather all X to top processes
    if (self.pZ == 0) {
        self.X = std::move(X);
        if (self.pY == 0) {
            X.resize(self.nX*Y_SIZE*Z_SIZE, 0);
            offsY.resize(self.pH, 0); cnts.resize(self.pH, 0);
            cnts[0] = self.nX*self.nY*Z_SIZE;
            for (int i = 1; i < self.pH; ++i) {
                cnts[i] = self.nX*self.pNY[i]*Z_SIZE;
                offsY[i] = offsY[i-1] + cnts[i-1];
            }
        }
        MPI_Gatherv(self.X.data(), self.nX*self.nY*Z_SIZE, MPI_T, X.data(), cnts.data(), offsY.data(), MPI_T, 0, self.commY);
    }

    // Gather all X to first process
    if (self.pZ == 0 && self.pY == 0) {
        self.X = std::move(X);
        if (self.pX == 0) {
            X.resize(X_SIZE*Y_SIZE*Z_SIZE, 0);
            offsX.resize(self.pW, 0); cnts.resize(self.pW, 0);
            cnts[0] = self.nX*Y_SIZE*Z_SIZE;
            for (int i = 1; i < self.pW; ++i) {
                cnts[i] = self.pNX[i]*Y_SIZE*Z_SIZE;
                offsX[i] = offsX[i-1] + cnts[i-1];
            }
        }
        MPI_Gatherv(self.X.data(), self.nX*Y_SIZE*Z_SIZE, MPI_T, X.data(), cnts.data(), offsX.data(), MPI_T, 0, self.commX);
    }

    // Calculate the position of each process first value
    offsP.resize(self.pW*self.pH*self.pD+1, 0);
    for (int x = 0; x < self.pW; ++x)
        for (int y = 0; y < self.pH; ++y)
            for (int z = 0; z < self.pD; ++z) {
                offsP[x*self.pD*self.pH + y*self.pD + z+1] = offsP[x*self.pD*self.pH + y*self.pD + z] + self.pNX[x]*self.pNY[y]*self.pNZ[z];
            }
    
    self.X = std::move(X);
}

T dot(const obj_s &self, const std::vector<T> &v1, const std::vector<T> &v2)
{
    T v = 0, res;
    for (size_t i = 0; i < self.matSize; ++i)
        v += v1[i] * v2[i];
    MPI_Allreduce(&v, &res, 1, MPI_T, MPI_SUM, self.comm3d);
    return res;
}

void axpy(std::vector<T> &out, T a, const std::vector<T> &x, const std::vector<T> &y)
{
    size_t size = out.size();
    for (size_t i = 0; i < size; ++i)
        out[i] = a*x[i] + y[i];
}

std::vector<T> matrixVector(const obj_s &self, const std::vector<T> &vec)
{
    std::vector<T> ret(self.matSize, 0);

    for (size_t z = 0; z < self.nZ; ++z)
        for (size_t y = 0; y < self.nY; ++y)
            for (size_t x = 0; x < self.nX; ++x)
                ret[pos(x,y,z)] = vec[posExt(x,y,z)]
                    - vec[posExt(x-1,y,z)]/6 - vec[posExt(x+1,y,z)]/6
                    - vec[posExt(x,y-1,z)]/6 - vec[posExt(x,y+1,z)]/6
                    - vec[posExt(x,y,z-1)]/6 - vec[posExt(x,y,z+1)]/6;
    return ret;
}

void shareVector(const obj_s &self, std::vector<T> &out, const std::vector<T> &in)
{
    MPI_Status s;
    // Exchange data on the row
    if (self.pX > 0 && self.pX < self.pW-1) {
        MPI_Sendrecv(&in[pos(0,0,0)],         1, self.inW, self.pX-1, 0, &out[posExt(self.nX,0,0)], 1, self.outW, self.pX+1, 0, self.commX, &s);
        MPI_Sendrecv(&in[pos(self.nX-1,0,0)], 1, self.inW, self.pX+1, 0, &out[posExt(-1,0,0)],      1, self.outW, self.pX-1, 0, self.commX, &s);
    } else if (self.pX > 0)
        MPI_Sendrecv(&in[pos(0,0,0)],         1, self.inW, self.pX-1, 0, &out[posExt(-1,0,0)],      1, self.outW, self.pX-1, 0, self.commX, &s);
    else if (self.pX < self.pW-1)
        MPI_Sendrecv(&in[pos(self.nX-1,0,0)], 1, self.inW, self.pX+1, 0, &out[posExt(self.nX,0,0)], 1, self.outW, self.pX+1, 0, self.commX, &s);

    // Exchange data on the column
    if (self.pY > 0 && self.pY < self.pH-1) {
        MPI_Sendrecv(&in[pos(0,0,0)],         1, self.inH, self.pY-1, 0, &out[posExt(0,self.nY,0)], 1, self.outH, self.pY+1, 0, self.commY, &s);
        MPI_Sendrecv(&in[pos(0,self.nY-1,0)], 1, self.inH, self.pY+1, 0, &out[posExt(0,-1,0)],      1, self.outH, self.pY-1, 0, self.commY, &s);
    } else if (self.pY > 0)
        MPI_Sendrecv(&in[pos(0,0,0)],         1, self.inH, self.pY-1, 0, &out[posExt(0,-1,0)],      1, self.outH, self.pY-1, 0, self.commY, &s);
    else if (self.pY < self.pH-1)
        MPI_Sendrecv(&in[pos(0,self.nY-1,0)], 1, self.inH, self.pY+1, 0, &out[posExt(0,self.nY,0)], 1, self.outH, self.pY+1, 0, self.commY, &s);

    // Exchange data on the depth-level
    if (self.pZ > 0 && self.pZ < self.pD-1) {
        MPI_Sendrecv(&in[pos(0,0,0)],         1, self.inD, self.pZ-1, 0, &out[posExt(0,0,self.nZ)], 1, self.outD, self.pZ+1, 0, self.commZ, &s);
        MPI_Sendrecv(&in[pos(0,0,self.nZ-1)], 1, self.inD, self.pZ+1, 0, &out[posExt(0,0,-1)],      1, self.outD, self.pZ-1, 0, self.commZ, &s);
    } else if (self.pZ > 0)
        MPI_Sendrecv(&in[pos(0,0,0)],         1, self.inD, self.pZ-1, 0, &out[posExt(0,0,-1)],      1, self.outD, self.pZ-1, 0, self.commZ, &s);
    else if (self.pZ < self.pD-1)
        MPI_Sendrecv(&in[pos(0,0,self.nZ-1)], 1, self.inD, self.pZ+1, 0, &out[posExt(0,0,self.nZ)], 1, self.outD, self.pZ+1, 0, self.commZ, &s);

    // Update out values
    for (size_t z = 0; z < self.nZ; ++z)
        for (size_t y = 0; y < self.nY; ++y)
            for (size_t x = 0; x < self.nX; ++x)
                out[posExt(x,y,z)] = in[pos(x,y,z)];
}

int main(int argc, char** argv)
{
    MPI_Init(&argc, &argv);
    try {
        obj_s self;
        self.start_all = std::chrono::system_clock::now();
        init(self);

        self.start = std::chrono::system_clock::now();

        self.diff = self.Y;
        self.dotDiff = dot(self, self.diff, self.diff);
        self.dir = self.diff;
        std::vector<T> extendedDir((self.nX+2)*(self.nY+2)*(self.nZ+2), 0);
        shareVector(self, extendedDir, self.dir);
        for (self.iter = 0; true; ++self.iter) {
            std::vector<T> Adir = matrixVector(self, extendedDir);

            T alpha = self.dotDiff / dot(self, self.dir, Adir);
            axpy(self.X, alpha, self.dir, self.X);
            axpy(self.diff, -alpha, Adir, self.diff);

            T oldDotdiff = self.dotDiff;
            self.dotDiff = dot(self, self.diff, self.diff);
            if (self.dotDiff < 1e-7 || self.iter >= MAX_ITER-1)
                break;

            T beta = self.dotDiff / oldDotdiff;
            axpy(self.dir, beta, self.dir, self.diff);
            shareVector(self, extendedDir, self.dir);
        }

        self.end = std::chrono::system_clock::now();

        std::vector<int> offsX, offsY, offsZ, offs;
        gatherX(self, offsX, offsY, offsZ, offs);

        self.end_all = std::chrono::system_clock::now();

        if (self.pId == 0) {
            std::cout << "Time: " << (double)(self.end - self.start).count() * std::chrono::system_clock::duration::period::num / std::chrono::system_clock::duration::period::den;
            std::cout << ",\tTotal time: " << (double)(self.end_all - self.start_all).count() * std::chrono::system_clock::duration::period::num / std::chrono::system_clock::duration::period::den;
            std::cout << ",\tIter: " << self.iter+1 << std::endl;
            /*int pz = 0;
            for (int z = 0; z < Z_SIZE; ++z) {
                if (z >= self.pOffZ[pz] + self.pNZ[pz]) ++pz;
                int py = 0;
                for (int y = 0; y < Y_SIZE; ++y) {
                    if (y >= self.pOffY[py] + self.pNY[py]) ++py;
                    int px = 0;
                    for (int x = 0; x < X_SIZE; ++x) {
                        if (x >= self.pOffX[px] + self.pNX[px]) ++px;
                        // Values are stored by process, with internal order kept
                        int p = offs[px*self.pD*self.pH + py*self.pD + pz] + // Process offset
                            (z-self.pOffZ[pz])*self.pNX[px]*self.pNY[py] + (y-self.pOffY[py])*self.pNX[px] + x-self.pOffX[px]; // Internal offset
                        std::cout << x << "," << y << "," << z << ": ";
                        std::cout << self.X[p] << "\t=\t" << cos(x)*cos(y)*cos(z) << std::endl;
                    }
                }
            }*/
        }
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        MPI_Finalize();
        return 1;
    }
    MPI_Finalize();
    return 0;
}