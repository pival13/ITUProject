#include <stdint.h>
#include <stdio.h>
#include <mpi.h>

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);
    int nproc, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &nproc);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    printf("%d %d\n", nproc, rank);
    MPI_Finalize();
    return 0;
}

// cl *.c /I "${ENV:MSMPI_INC}\" /link "${ENV:MSMPI_LIB64}\msmpi.lib"
// mpiexec /np [nbProcess] *.exe