#include <iostream>
#include <mpi.h>

int main (int argc, char* argv[])
{
    if (MPI_Init (&argc, argv) != MPI_SUCCESS)
    {
        std::cout << "MPI is not working";
        MPI_Abort();
    }
    std::cout << "Hello world!" << std::endl;
    MPI_Finalize();
}