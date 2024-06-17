#include <iostream>
#include <fstream>
#include <string>
#include <mpi.h>


void check(int rc) {
    if (rc != MPI_SUCCESS){
    std::cout << "MPI eroor: " << rc;
    MPI_Abort(MPI_COMM_WORLD, rc);
    }
}


int main(int argc, char* argv[])
{
    int N = 120;
    int rank, size, count;
    int buf[N];
    int rc;
    MPI_Status status;
    rc = MPI_Init(&argc, &argv);
    check(rc);
    rc = MPI_Comm_size (MPI_COMM_WORLD, &size);
    check(rc);
    rc = MPI_Comm_rank (MPI_COMM_WORLD, &rank);
    check(rc);
    rc = MPI_Get_count(&status, MPI_INT, &count);
    check(rc);
    if (N % size != 0)
    {
        MPI_Abort(MPI_COMM_WORLD, 1);
        std::cerr << "N is not divisible by size" << std::endl;
    }
    int gath_buf[N / size];
    int start_point = N / size * rank;
    for (int i  = 0; i < N / size; ++i)
    {
        gath_buf[i] = rank;
    }
    rc = MPI_Gather(gath_buf, N/size, MPI_INT, buf + start_point,  N/size, MPI_INT, 0, MPI_COMM_WORLD);
    check(rc);
    MPI_Finalize();
    if (rank == 0)
	{
		std::fstream data;
		data.open("gather.dat", std::ios::out);
		for (int i = 0; i < N; ++i)
			data << buf[i] << " ";
		data.close();
	}
    return 0;
}