#include <mpi.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <random>


int sigflag = 1;
void handler(int nsig)
{
    sigflag = 0;
}


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
    signal(SIGALRM, handler);
    const int REPEAT = 3;
    int rank, size, count;
    int rc;
    check(MPI_Init(&argc, &argv));
    check(MPI_Comm_size(MPI_COMM_WORLD, &size));
    check(MPI_Comm_rank(MPI_COMM_WORLD, &rank));
    MPI_Status status;
    srandom(rank);
    int msg_num = 0;
    char msg = '0';
    int request_status = 0;
    if (rank == 0)
    {
        check(MPI_Send(&msg, 1, MPI_CHAR, (rank + 1) % size, 't', MPI_COMM_WORLD));
    }
    int is_flag = 0;
    for (int i = 0; i < REPEAT; ++i)
    {
        MPI_Recv(&msg, 1, MPI_CHAR, MPI_ANY_SOURCE, 't', MPI_COMM_WORLD, &status);
        printf("rank %d received marker, entered critical section, num = %d\n", rank, i + 1);
        sleep(random() % 2 + 1);
        check(MPI_Send(&msg, 1, MPI_CHAR, (rank + 1) % size, 't', MPI_COMM_WORLD));
        printf("rank %d sent marker, exited critical section\n", rank);
        alarm(random() % 5 + 1);
        printf("rank %d entered remainder section\n", rank);
        while (sigflag)
        {
            check(MPI_Iprobe(MPI_ANY_SOURCE, 't', MPI_COMM_WORLD, &is_flag, &status));
            if (is_flag == 1)
            {
                check(MPI_Recv(&msg, 1, MPI_CHAR, MPI_ANY_SOURCE, 't', MPI_COMM_WORLD, &status));
                check(MPI_Send(&msg, 1, MPI_CHAR, (rank + 1) % size, 't', MPI_COMM_WORLD));
            }
        }
        printf("rank %d exited remainder section\n", rank);
    }
    printf("rank %d\t finalize sections\n", rank);
    int sent_fin = 2;
    if (rank == 0)
    {
        check(MPI_Send(&msg, 1, MPI_CHAR, (rank + 1) % size, 'f', MPI_COMM_WORLD));
        printf("rank %d\t sent fin\n", rank);
        --sent_fin;
    }
    while (sent_fin)
    {
        check(MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &is_flag, &status));
        if (is_flag == 1)
        {
            if (status.MPI_TAG == 't')
            {
                check(MPI_Recv(&msg, 1, MPI_CHAR, MPI_ANY_SOURCE, 't', MPI_COMM_WORLD, &status));
                check(MPI_Send(&msg, 1, MPI_CHAR, (rank + 1) % size, 't', MPI_COMM_WORLD));
            }
            else
            {
                check(MPI_Recv(&msg, 1, MPI_CHAR, MPI_ANY_SOURCE, 'f', MPI_COMM_WORLD, &status));
                printf("rank %d\t received fin\n", rank);
                check(MPI_Send(&msg, 1, MPI_CHAR, (rank + 1) % size, 'f', MPI_COMM_WORLD));
                printf("rank %d\t sent fin\n", rank);
                --sent_fin;
            }
        }
    }
    printf("rank %d\t finalized\n", rank);
    MPI_Finalize();
    return 0;
}
