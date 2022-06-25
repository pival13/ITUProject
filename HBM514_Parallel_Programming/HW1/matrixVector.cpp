#include <mpi.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <tuple>
#include <sstream>

#define MAT_TYPE uint32_t
#define MPI_TYPE MPI_UINT32_T

typedef struct {
    MPI_Comm comm;
    int rank;
} Group;

typedef struct {
    Group graph;
    Group row;
    Group col;
    int nproc1D;
    MPI_Datatype colType, subMatType;
    size_t matSize;
} Container;

Container initializeProcess(const char *matSize)
{
    Container c;
    int nproc;
    MPI_Comm_size(MPI_COMM_WORLD, &nproc);
    if (nproc == 1) c.nproc1D = 1;
    else if (nproc == 4) c.nproc1D = 2;
    else if (nproc == 16) c.nproc1D = 4;
    else if (nproc == 64) c.nproc1D = 8;
    else if (nproc == 144) c.nproc1D = 12;
    else
        throw std::invalid_argument("A square number of processor is required");
    c.matSize = std::stoull(matSize);
    if (c.matSize % c.nproc1D != 0)
        throw std::invalid_argument("The matrix size must be divisible by the square root of the number of process.");

    MPI_Cart_create(MPI_COMM_WORLD, 2, std::vector<int>{c.nproc1D,c.nproc1D}.data(), std::vector<int>{0,0}.data(), 1, &c.graph.comm);
    MPI_Comm_rank(c.graph.comm, &c.graph.rank);

    MPI_Cart_sub(c.graph.comm, std::vector<int>{0,1}.data(), &c.row.comm);
    MPI_Comm_rank(c.row.comm, &c.row.rank);

    MPI_Cart_sub(c.graph.comm, std::vector<int>{1,0}.data(), &c.col.comm);
    MPI_Comm_rank(c.col.comm, &c.col.rank);

    MPI_Type_vector(c.matSize, c.matSize/c.nproc1D, c.matSize, MPI_TYPE, &c.colType);
    MPI_Type_commit(&c.colType);

    if (c.matSize/c.nproc1D * c.matSize/c.nproc1D <= INT32_MAX)
        MPI_Type_contiguous(c.matSize/c.nproc1D * c.matSize/c.nproc1D, MPI_TYPE, &c.subMatType);
    else // Beware of overflow
        MPI_Type_vector(c.matSize/c.nproc1D, c.matSize/c.nproc1D, c.matSize/c.nproc1D, MPI_TYPE, &c.subMatType);
    MPI_Type_commit(&c.subMatType);

    return c;
}

std::tuple<std::vector<MAT_TYPE>,std::vector<MAT_TYPE>> exchangeInitData(const Container &c, const std::vector<MAT_TYPE> &fullMatrix, const std::vector<MAT_TYPE> &fullVector)
{
    MPI_Request req;
    MPI_Status stat;

    int newSize = c.matSize/c.nproc1D;
    std::vector<MAT_TYPE> matrix(newSize * newSize);
    std::vector<MAT_TYPE> vector(newSize);
    std::vector<MAT_TYPE> tmp;

    // Emit column matrix and subvector to the elements of the first row
    if (c.graph.rank == 0)
        for (int i = 0; i < c.nproc1D; ++i) {
            MPI_Isend(fullMatrix.data() + newSize*i, 1, c.colType, i, 0, c.row.comm, &req);
            MPI_Isend(fullVector.data() + newSize*i, newSize, MPI_TYPE, i, 0, c.row.comm, &req);
        }

    if (c.col.rank == 0) {
        // Receive column matrix and subvector on the head of each column, that is the elements of first row
        tmp.resize(newSize * c.matSize);
        MPI_Recv(tmp.data(), newSize*c.matSize, MPI_TYPE, 0, 0, c.row.comm, &stat);
        MPI_Recv(vector.data(), newSize, MPI_TYPE, 0, 0, c.row.comm, &stat);
    }

    // Disperse the column matrix to every process on the column
    MPI_Scatter(tmp.data(), 1, c.subMatType, matrix.data(), newSize*newSize, MPI_TYPE, 0, c.col.comm);
    // Share the vector to every process on the column
    MPI_Bcast(vector.data(), newSize, MPI_TYPE, 0, c.col.comm);

    return {matrix, vector};
}

std::vector<MAT_TYPE> exchangeFinalData(const Container &c, const std::vector<MAT_TYPE> &vector)
{
    // Retrieve the result on the rows and add them together
    std::vector<MAT_TYPE> results;
    if (c.row.rank == 0)
        results.resize(vector.size());
    MPI_Reduce(vector.data(), results.data(), vector.size(), MPI_TYPE, MPI_SUM, 0, c.row.comm);

    // Gather all subvectors on the main process
    std::vector<MAT_TYPE> finalVector;
    if (c.graph.rank == 0)
        finalVector.resize(c.matSize);
    if (c.row.rank == 0)
        MPI_Gather(results.data(), results.size(), MPI_TYPE, finalVector.data(), results.size(), MPI_TYPE, 0, c.col.comm);
    return finalVector;
}

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);
    try {
        if (argc != 2)
            throw std::invalid_argument(argv[0] + std::string(" takes one argument, the matrix size."));

        Container c = initializeProcess(argv[1]);
        std::vector<MAT_TYPE> matrix, vector;

        // Fill matrix and vector
        if (c.graph.rank == 0) {
            matrix.resize(c.matSize*c.matSize);
            for (size_t i = 0; i < c.matSize*c.matSize; ++i)
                matrix[i] = (MAT_TYPE)i;
            vector.resize(c.matSize);
            for (size_t i = 0; i < c.matSize; ++i)
                vector[i] = (MAT_TYPE)i;
        }

        auto [subMatrix, subVector] = exchangeInitData(c, matrix, vector);

        // Perform multiplication
        std::vector<MAT_TYPE> result(subVector.size(), 0);
        for (size_t i = 0; i < subVector.size(); ++i)
            for (size_t j = 0; j < subVector.size(); ++j)
                result[j] += subMatrix[subVector.size()*j + i] * subVector[i];

        vector = exchangeFinalData(c, result);

        if (c.graph.rank == 0) {
            std::stringstream ss;
            for (size_t i = 0; i < vector.size(); ++i)
                ss << vector[i] << "\t";
            std::cout << ss.str() << std::endl;
        }

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