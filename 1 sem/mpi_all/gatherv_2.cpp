#include <iostream>
#include <fstream>
#include <string>
#include <cmath>
#include <stdlib.h>
#include <mpi.h>

// 

void check(int rc) {
    if (rc != MPI_SUCCESS){
    std::cout << "MPI eroor: " << rc;
    MPI_Abort(MPI_COMM_WORLD, rc);
    }
}


int main(int argc, char* argv[])
{
    int rank, size, count;
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

    srandom(rank + 1);
    int buf_len = random() % 7 + 1;
    // std::cout << buf_len  << " " << rank << std::endl;
    int kol = buf_len;
    double gath_buf[buf_len];
    int full_len = 0;
    int recvcounts[size];
    int displs[size];
    for (int i  = 0; i < buf_len; ++i)
        gath_buf[i] = sqrt(rank);
    for (int i = 0; i < size; ++i)
        recvcounts[i] = 1;
    rc = MPI_Gather(&kol, 1, MPI_INT, recvcounts, 1, MPI_INT, 0, MPI_COMM_WORLD);
    check(rc);
    if (rank == 0)
    {
        for (int i = 0; i < size; ++i)
        {
            if (i == 0)
                displs[i] = 0;
            else
                displs[i] = displs[i - 1] + recvcounts[i - 1];
        }
        full_len = recvcounts[size - 1] + displs[size - 1];
    }
    double full_arr[full_len];
    for(int i = 0; i < full_len; ++i)
        full_arr[i] = 0;

    rc = MPI_Gatherv(gath_buf, kol, MPI_DOUBLE, full_arr, 
            recvcounts, displs, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    check(rc);
    MPI_Finalize();
    if (rank == 0)
	{
		std::fstream data;
		data.open("gatherv_2.dat", std::ios::out);
		for (int i = 0; i < full_len; ++i)
        {
            if (full_arr[i] != full_arr[i - 1] && i > 0)
                data << "\n";
            data << full_arr[i] << " ";
        }
        data.close();
	}
    return 0;
}