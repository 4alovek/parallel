#include <mpi.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <random>


void check(int rc)
{
    if (rc != MPI_SUCCESS)
    {
        std::cerr << "MPI eroor: " << rc;
        MPI_Abort(MPI_COMM_WORLD, rc);
    }
}


int main(int argc, char *argv[])
{
    const int REPEAT = 3;
    int rank, size, count;
    int rc;
    check(MPI_Init(&argc, &argv));
    check(MPI_Comm_size(MPI_COMM_WORLD, &size));
    check(MPI_Comm_rank(MPI_COMM_WORLD, &rank));
    MPI_Status status;
    srandom(rank);
    int msg_num = 0;
    int msg = 0;
    int request_status = 0;
    if (rank == 0)
    {
        while (msg_num < (size - 1) * REPEAT)
        {
            if (request_status == 0)
            {
                MPI_Recv(&msg, 1, MPI_INT, MPI_ANY_SOURCE, 10, MPI_COMM_WORLD, &status);
                printf("rank %d\t received request\n", rank);
                request_status = 1;
                MPI_Send(&msg, 1, MPI_INT, status.MPI_SOURCE, 1, MPI_COMM_WORLD);
                printf("rank %d\t sent permit to %d\n", rank, status.MPI_SOURCE);
                MPI_Recv(&msg, 1, MPI_INT, status.MPI_SOURCE, 11, MPI_COMM_WORLD, &status);
                printf("rank %d\t received release\n", rank);
                ++msg_num;
                request_status = 0;
                printf("msg_left = %d\n", (size - 1) * REPEAT - msg_num);
            }
            
        }
    }
    else
    {
        int crit_duration = random() % 5 + 1;
        int rem_duration = random() % 5 + 1;
        for (int i = 0; i < REPEAT; ++i)
        {
            MPI_Send(&msg, 1, MPI_INT, 0, 10, MPI_COMM_WORLD);
            printf("rank %d\t sent request\n", rank);
            MPI_Recv(&msg, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            printf("rank %d\t received permit\n", rank);
            sleep(crit_duration);
            MPI_Send(&msg, 1, MPI_INT, 0, 11, MPI_COMM_WORLD);
            printf("rank %d\t sent release\n", rank);
            sleep(rem_duration);
        }
    }
    printf("rank %d\t finalize\n", rank);
    MPI_Finalize();
    return 0;
}
