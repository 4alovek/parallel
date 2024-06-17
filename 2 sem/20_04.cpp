#include <mpi.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <random>


const char VOTE = 'v';
const char OK = 'k';
const char COORD = 'c';


int sigflag = 0;
void handler(int nsig)
{
    sigflag = 1;
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
    int RANK = 1;
    unsigned int TIMEOUT = 2;
    int rank, size, count;
    int rc;
    rc = MPI_Init(&argc, &argv);
    rc = MPI_Comm_size(MPI_COMM_WORLD, &size);
    check(rc);
    rc = MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    check(rc);
    MPI_Status status;
    srandom(rank);
    int is_fall = random();
    if ((rank != RANK) && (is_fall % 2 == 0))
    {
        printf("Process %d died\n", rank);
        MPI_Finalize();
        return (0);
    }
    std::vector<int> process_list(size, 0);
    // for (int i = 0; i < size; ++i)
    //     printf("%d\t", process_list[i]);
    // printf("\n");
    bool have_coord = 0;

    int next = (rank + 1) % size;
    if (rank == RANK)
        alarm(TIMEOUT);
    while (!have_coord)
    {
        if (sigflag)
        {
            printf("rank %d didnt wait for %d\n", rank, next);
            next = (next + 1) % size;
            check(MPI_Send(process_list.data(), size, MPI_INT, next, VOTE, MPI_COMM_WORLD));
            printf("rank %d sent vote to %d\n", rank, next);
            sigflag = 0;
            alarm(TIMEOUT);
        }
        int is_flag = 0;
        int v_status = 0;
        check(MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &is_flag, &status));
        if (is_flag == 1)
        {
            check(MPI_Recv(process_list.data(), size, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status));
            if (status.MPI_TAG == VOTE)
            {
                printf("rank %d received voting\n", rank);
                check(MPI_Send(&OK, 1, MPI_CHAR, status.MPI_SOURCE, OK, MPI_COMM_WORLD));
                printf("rank %d sent ok to %d\n", rank, status.MPI_SOURCE);
                if (process_list[rank] != 0)
                {
                    // for (int i = 0; i < size; ++i)
                        // printf("%d\t", process_list[i]);
                    // printf("\n");
                    printf("rank %d finding coordinator\n", rank);
                    int coordinator = rank;
                    for (int i = 0; i < size; ++i)
                    {
                        if (process_list[i] != 0)
                        {
                            coordinator = i;
                            printf("rank %d found coordinator %d\n", rank, coordinator);
                        }
                    }
                    process_list[0] = coordinator;
                    process_list[1] = rank;
                    check(MPI_Send(process_list.data(), size, MPI_INT, next, COORD, MPI_COMM_WORLD));
                }
                else
                {
                    process_list[rank] = 1;
                    // check(MPI_Send(&OK, 1, MPI_INT, status.MPI_SOURCE, OK, MPI_COMM_WORLD));
                    // printf("rank %d sent ok to %d\n", rank, status.MPI_SOURCE);
                    check(MPI_Send(process_list.data(), size, MPI_INT, next, VOTE, MPI_COMM_WORLD));
                    printf("rank %d sent vote to %d\n", rank, next);
                    sigflag = 0;
                    alarm(TIMEOUT);
                }
            }
            else if (status.MPI_TAG == OK)
            {
                printf("rank %d received ok from %d\n", rank, status.MPI_SOURCE);
                alarm(0);
            }
            else
            {
                printf("rank %d received coordinator from %d, coordinator is %d\n", rank, status.MPI_SOURCE, process_list[0]);
                have_coord = 1;
                if (process_list[1] != rank)
                {
                    check(MPI_Send(&OK, 1, MPI_CHAR, status.MPI_SOURCE, OK, MPI_COMM_WORLD));
                    printf("rank %d sent ok to %d\n", rank, status.MPI_SOURCE);
                    check(MPI_Send(process_list.data(), size, MPI_INT, next, COORD, MPI_COMM_WORLD));
                    printf("rank %d sent coord\n", rank);
                }
            }
        }
    }
    MPI_Finalize();
    return 0;
}
