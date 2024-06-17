#include <mpi.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <iostream>
#include <fstream>
#include <random>

const char VOTE = 'v';
const char OK = 'k';
const char COORD = 'c'; 

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

void vote_start(int rank, int size)
{
    printf("rank %d started voting\n", rank);
    for (int i = rank + 1; i < size; ++i)
    {
        check(MPI_Send(&VOTE, 1, MPI_CHAR, i, 20, MPI_COMM_WORLD));
    }
    printf("rank %d sent messages\n", rank);
}

int vote_check(int rank, MPI_Status status, int msg)
{
    int v_status = 0;
    if (msg == VOTE)
    {
        printf("rank %d received voting\n", rank);
        check(MPI_Send(&OK, 1, MPI_CHAR, status.MPI_SOURCE, 20, MPI_COMM_WORLD));
        printf("rank %d sent ok to %d\n", rank, status.MPI_SOURCE);
    }
    else if (msg == OK)
    {
        v_status = 1;
        printf("rank %d received ok from %d\n", rank, status.MPI_SOURCE);
    }
    else if (msg == COORD)
    {
        v_status = 2;
        printf("rank %d received coordinator from %d\n", rank, status.MPI_SOURCE);
    }
    return v_status;
}

void vote_coordinator(int rank, int size)
{
    printf("rank %d now coordinator\n", rank);
    for (int i = 0; i < size; ++i)
    {
        check(MPI_Send(&COORD, 1, MPI_CHAR, i, 20, MPI_COMM_WORLD));
    }
    printf("rank %d sent coord\n", rank);
}

// при N=6 координатор=3, N=9 координатор=8
int main(int argc, char *argv[])
{
    signal(SIGALRM, handler);
    int RANK = 1;
    unsigned int TIMEOUT = 5;
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
    if ((rank != RANK) && (is_fall % 2 != 0))
    {
        printf("Process %d died\n", rank);
        MPI_Finalize();
        return(0);
    }
    bool is_initiator = 0;
    bool was_initiator = 0;
    if (rank == RANK)
        is_initiator = 1;

    while (true)
    {
        int is_flag = 0;
        int buf_msg = 0;
        int v_status = 0;
        if (is_initiator == 1 && was_initiator == 0)
        {
            vote_start(rank, size);
            alarm(TIMEOUT);
            while (sigflag)
            {
                check(MPI_Iprobe(MPI_ANY_SOURCE, 20, MPI_COMM_WORLD, &is_flag, &status));
                if (is_flag == 1)
                {
                    check(MPI_Recv(&buf_msg, 1, MPI_INT, MPI_ANY_SOURCE, 20, MPI_COMM_WORLD, &status));
                    v_status = vote_check(rank, status, buf_msg);
                    if (v_status)
                    {
                        alarm(0);
                        break;
                    }
                }
            }
            if (v_status == 0)
            {
                vote_coordinator(rank, size);
            }
            is_initiator = 0;
            was_initiator = 1;
        }
        else
        {
            // printf("ya ustal %d", rank);
            check(MPI_Iprobe(MPI_ANY_SOURCE, 20, MPI_COMM_WORLD, &is_flag, &status));
            if (is_flag == 1)
                {
                    check(MPI_Recv(&buf_msg, 1, MPI_INT, MPI_ANY_SOURCE, 20, MPI_COMM_WORLD, &status));
                    v_status = vote_check(rank, status, buf_msg);
                    if (v_status == 0)
                    {
                        is_initiator = 1;
                    }
                }
        }
        if (v_status == 2)
        {
            break;
        }
    }
    MPI_Finalize();
    return 0;
}
