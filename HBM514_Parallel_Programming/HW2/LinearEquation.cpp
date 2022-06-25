#include "LinearEquation.h"

#if defined(DIRECT)
#   include "LU.cpp"
#elif defined(JACOBI)
#   include "jacobi.cpp"
#elif defined(GS)
#   include "gaussSeidel.cpp"
#else
#   error One of DIRECT, JACOBI or GC must be defined
#endif

static void initialize(obj_s &self)
{
    MPI_Comm_size(MPI_COMM_WORLD, &self.pNum);
    MPI_Comm_rank(MPI_COMM_WORLD, &self.pId);

    if (self.pId == 0) {
        self.A.resize(MAT_SIZE*MAT_SIZE);
        self.B.resize(MAT_SIZE);
        for (size_t i = 0; i < MAT_SIZE; ++i) {
            self.A[i*MAT_SIZE + i] = float(rand() % 10 + 1) * 100;
            for (size_t j = i+1; j < MAT_SIZE; ++j)
                self.A[i*MAT_SIZE + j] = self.A[j*MAT_SIZE + i] = std::cos(float(rand()));
            self.B[i] = 10 * std::cos(float(rand()));
        }
    }
    self.end4 = std::chrono::system_clock::time_point::min();
}

static void display(obj_s &self)
{
    #if defined(DIRECT)
        std::cout << "Time: " << (double)(self.end7 - self.start).count() * std::chrono::system_clock::duration::period::num / std::chrono::system_clock::duration::period::den << std::endl;
    #else
        std::cout << "Time: 1e-4: " << (double)(self.end4 - self.start).count() * std::chrono::system_clock::duration::period::num / std::chrono::system_clock::duration::period::den;
        std::cout << ", 1e-7: " << (double)(self.end7 - self.start).count() * std::chrono::system_clock::duration::period::num / std::chrono::system_clock::duration::period::den << std::endl;
        std::cout << "Iter: 1e-4: " << self.iter4 << ", 1e-7: " << self.iter << std::endl;
    #endif
    // for (size_t i = 0; i < MAT_SIZE; ++i) {
    //     //for (size_t j = 0; j < MAT_SIZE; ++j)
    //     //    std::cout << self.A[i*MAT_SIZE + j] << "\t";
    //     std::cout << "*\t" << self.X[i]/* << "\t=\t" << self.B[i]*/ << std::endl;
    // }
}

int main(int argc, char **argv)
{
    srand(RAND_SEED);
    MPI_Init(&argc, &argv);
    try {
        obj_s obj;
        initialize(obj);

        if (obj.pId == 0) obj.start = std::chrono::system_clock::now();
#       if defined(DIRECT)
            LU(obj);
#       elif defined(JACOBI)
            jacobi(obj);
#       elif defined(GS)
            gaussSeidel(obj);
#       endif
        if (obj.pId == 0) obj.end7 = std::chrono::system_clock::now();

        if (obj.pId == 0)
            display(obj);
    } catch (const std::exception &e) {
        int r;
        MPI_Comm_rank(MPI_COMM_WORLD, &r);
        if (r == 0)
            std::cerr << e.what() << std::endl;
        MPI_Finalize();
        return 1;
    }
    MPI_Finalize();
    return 0;
}