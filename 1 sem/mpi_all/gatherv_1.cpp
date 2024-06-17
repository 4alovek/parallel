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
    // int N = 120;
    int rank, size, count;
    // int buf[N];
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
    int gath_buf[rank + 1];
    int recvcounts[size];
    int displs[size];
    for (int i  = 0; i < rank + 1; ++i)
        gath_buf[i] = rank + 1;
    for (int i = 0; i < size; ++i)
        recvcounts[i] = i + 1;
    for (int i = 0; i < size; ++i)
    {
        if (i == 0)
            displs[i] = 0;
        else
            displs[i] = displs[i - 1] + i * 2;
        std::cout << displs[i] << std::endl;
    }
    int full_len = size + displs[size - 1];
    int full_arr[full_len];
    for(int i = 0; i < full_len; ++i)
        full_arr[i] = 0;
    rc = MPI_Gatherv(gath_buf, rank+1, MPI_INT, full_arr, 
            recvcounts, displs, MPI_INT, 0, MPI_COMM_WORLD);
    check(rc);
    MPI_Finalize();
    if (rank == 0)
	{
		std::fstream data;
		data.open("gatherv_1.dat", std::ios::out);
		for (int i = 0; i < full_len; ++i)
        {
            // if (full_arr[i] != 0 && full_arr[i-1] == 0)
            //     data << "\n";
        	data << full_arr[i] << " ";
        }
        data.close();
	}
    return 0;
}