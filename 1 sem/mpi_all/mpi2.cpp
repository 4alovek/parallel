#include <iostream>
#include <string>
#include <mpi.h>

void check(int rc) {
    if (rc != MPI_SUCCESS){
        std::cout << "MPI eroor: " << rc;
        MPI_Abort(MPI_COMM_WORLD, rc);
    }
}


int main (int argc, char* argv[])
{
    int rank, size, count;
    int buf = 0;
    int i = 0;
    int rc;
    MPI_Status status;
    rc = MPI_Init(&argc, &argv);
    check(rc);
    rc = MPI_Comm_size (MPI_COMM_WORLD, &size);
    check(rc);
    rc = MPI_Comm_rank (MPI_COMM_WORLD, &rank);
    check(rc);
    if (rank == 0) {
        int send_message = MPI_Send(&buf, 1, MPI_INT, (rank + 1) % size,
                                    99, MPI_COMM_WORLD);
        check(send_message);
        std::cout << "Rank: " << rank << '\t'  << "send: " << buf <<
                  '\t' << "to: " << (rank + 1) % size << "\n";
        int receive_message = MPI_Recv(&buf, 1, MPI_INT, MPI_ANY_SOURCE,
                                       MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        check(receive_message);
        MPI_Get_count(&status, MPI_INT, &count);
        std::cout << "Rank: " << rank << '\t'  << "received: " << buf
                  << '\t' << "from: " << status.MPI_SOURCE << "\n";
    }
    else {
        int receive_message = MPI_Recv(&buf, 1, MPI_INT, MPI_ANY_SOURCE,
                                        MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        check(receive_message);
        MPI_Get_count(&status, MPI_INT, &count);
        std::cout << "Rank: " << rank << '\t'  << "received: " << buf
                  << '\t' << "from: " << status.MPI_SOURCE << "\n";  
        buf += rank;
        int send_message = MPI_Send(&buf, 1, MPI_INT, (rank + 1) % size,
                                    99, MPI_COMM_WORLD);
        check(send_message);   
        std::cout << "Rank: " << rank << '\t'  << "send: " << buf <<
                  '\t' << "to: " << (rank + 1) % size << "\n";
    }
    MPI_Finalize();
    return 0;
}

