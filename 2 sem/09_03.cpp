#include <mpi.h>
#include <stdlib.h>
#include <iostream>

void check(int rc) {
    if (rc != MPI_SUCCESS){
    std::cout << "MPI eroor: " << rc;
    MPI_Abort(MPI_COMM_WORLD, rc);
    }
}


int main(int argc, char *argv[])
{
    int rank, size, count;
    int rc;
    rc = MPI_Init(&argc, &argv);

    rc = MPI_Comm_size (MPI_COMM_WORLD, &size);
    check(rc);
    rc = MPI_Comm_rank (MPI_COMM_WORLD, &rank);
    check(rc);
    MPI_Status status;

    srandom(rank);

    if (rank == 0)
    {
        int buf[2];
        int utc_time = random() % 21;
        int cli_time, d;
        for (int i = 0; i < size - 1; ++i)
        {
            rc = MPI_Recv(&cli_time, 1, MPI_INT, MPI_ANY_SOURCE, 1, MPI_COMM_WORLD, &status);
            printf("received from %d\t time: %d\n", status.MPI_SOURCE, utc_time);
            utc_time += random() % 21;
            d = random() % 4;
            utc_time += d;
            buf[0] = utc_time;
            buf[1] = d;
            rc = MPI_Send(&buf, 2, MPI_INT, status.MPI_SOURCE, 2, MPI_COMM_WORLD);
            printf("send to %d\t time: %d\t d: %d\n", status.MPI_SOURCE, utc_time, d);
        }
    }
    else
    {   
        int buf[2];
        int utc_time, d, time_0, time_1;
        int local_time = random() % 21;
        time_0 = local_time;
        // printf("Im alive");
        MPI_Send(&local_time, 1, MPI_INT, 0, 1, MPI_COMM_WORLD);
        MPI_Recv(&buf, 2, MPI_INT, 0, 2, MPI_COMM_WORLD, &status);
        utc_time = buf[0];
        d = buf[1];
        time_1 = local_time + 5 + random() % 6;
        local_time = utc_time + (time_1 - time_0 - d + 1) / 2;
        printf("T_0: %d\t T_1: %d corrected_time: %d\t received T_utc: %d\t d: %d\t rank: %d\n",
                time_0, time_1, local_time, utc_time, d, rank);
    }
    MPI_Finalize();
    return 0;
}
