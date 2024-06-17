#include <mpi.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
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
        std::cerr << "error " << rc;
        MPI_Abort(MPI_COMM_WORLD, rc);
    }
}

int main(int argc, char *argv[])
{
    signal(SIGALRM, handler);
    int RANK = 1;
    int rank, size;
    check(MPI_Init(&argc, &argv));
    check(MPI_Comm_rank(MPI_COMM_WORLD, &rank));
    check(MPI_Comm_size(MPI_COMM_WORLD, &size));
    srandom(rank);
    int flag_dead = random();
    if ((flag_dead % 2) && (rank != RANK))
    {
        printf("Rank %d dead\n", rank);
        MPI_Finalize();
        return 0;
    }
    int flag_vote = 0;
    if (rank == RANK)
        flag_vote++;
    // flag_vote = 0 -> Синий
    // flag_vote = 1 -> Красный
    // flag_vote = 2 -> Был красным
    const char vote = 'v';
    const char ok = 'o';
    const char coordinator = 'c'; 
    int TIMEOUT = 5;
    int flag_msg;
    bool flag_will_be_coord = 1;
    char recv_msg;
    MPI_Status status;
    while (true)
    {
        if (flag_vote == 1)
        {
            for (int i = rank + 1; i < size; i++)
            {
                check(MPI_Send(&vote, 1, MPI_CHAR, i, 1, MPI_COMM_WORLD));
                printf("Process %d send voting to %d\n", rank, i);
            }
            alarm(TIMEOUT);
            while (sigflag)
            {
                check(MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag_msg, &status));
                if (flag_msg)
                {
                    check(MPI_Recv(&recv_msg, 1, MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status));
                    if (recv_msg == vote)
                    {
                        printf("Process %d received voting from %d\n", rank, status.MPI_SOURCE);
                        check(MPI_Send(&ok, 1, MPI_CHAR, status.MPI_SOURCE, 1, MPI_COMM_WORLD));
                        printf("Process %d sent ok to %d\n", rank, status.MPI_SOURCE);
                    }
                    else
                    {
                        if (recv_msg == ok)
                            printf("Process %d received ok from %d\n", rank, status.MPI_SOURCE);
                        if (recv_msg == coordinator)
                            printf("Process %d received coordinator from %d\n", rank, status.MPI_SOURCE);
                        flag_will_be_coord = 0;
                        alarm(0);
                        break;
                    }
                }
            }
            if (flag_will_be_coord)
            {
                printf("Process %d is new coordinator\n", rank);
                for (int i = 0; i < size; i++)
                {
                    check(MPI_Send(&coordinator, 1, MPI_CHAR, i, 1, MPI_COMM_WORLD));
                    printf("Process %d sent coord to %d\n", rank, i);
                }
            }
            flag_vote++;
        }
        else
        {
            check(MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag_msg, &status));
            if (flag_msg)
            {
                check(MPI_Recv(&recv_msg, 1, MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status));
                if (recv_msg == vote)
                {
                    printf("Process %d received voting from %d\n", rank, status.MPI_SOURCE);
                    check(MPI_Send(&ok, 1, MPI_CHAR, status.MPI_SOURCE, 1, MPI_COMM_WORLD));
                    printf("Process %d sent ok to %d\n", rank, status.MPI_SOURCE);
                    if (flag_vote == 0)
                        flag_vote = 1;
                }
                else
                {
                    if (recv_msg == ok)
                        printf("Process %d received ok from %d\n", rank, status.MPI_SOURCE);
                    if (recv_msg == coordinator)
                    {
                        printf("Process %d received coordinator from %d\n", rank, status.MPI_SOURCE);
                        break;
                    }
                }
            }
        }
    }
    MPI_Finalize();
    return 0;
}