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
        int buf = 0;
        int full_disp = 0;
        int utc_time = random() % 51;
        int cli_time[size];
        cli_time[0] = utc_time;
        for (int i = 1; i < size; ++i)
        {
            rc = MPI_Send(&utc_time, 1, MPI_INT, i, 1, MPI_COMM_WORLD);
            printf("server:\tsend to %d: time = %d\n", i, utc_time);
        }

        for (int i = 0; i < size - 1; ++i)
        {
            rc = MPI_Recv(&buf, 1, MPI_INT, MPI_ANY_SOURCE, 2, MPI_COMM_WORLD, &status);
            printf("server:\treceived from %d\t delta: %d\n", status.MPI_SOURCE, buf);
            cli_time[status.MPI_SOURCE] = utc_time + buf;
            full_disp += buf;
        }
        if (full_disp > 0)
            full_disp = (full_disp + size - 1) / size;
        else
            full_disp = (full_disp - 1) / size;

        utc_time = utc_time + full_disp;
        for (int i = 1; i < size; ++i)
        {
            buf = utc_time - cli_time[i];
            rc = MPI_Send(&buf, 1, MPI_INT, i, 1, MPI_COMM_WORLD);
            printf("send to: %d, delta = %d\n", i, buf);
        }
        printf("new_time = %d\t rank = %d\n", utc_time, rank);
    }
    else
    {   
        int buf = 0;
        int utc_time = 0;
        int local_time = random() % 51;
        rc = MPI_Recv(&utc_time, 2, MPI_INT, 0, 1, MPI_COMM_WORLD, &status);
        check(rc);
        // printf("received from: %d, utc_time = %d\n", 0, utc_time);
        int delta = local_time- utc_time;
        rc = MPI_Send(&delta, 1, MPI_INT, 0, 2, MPI_COMM_WORLD);
        check(rc);
        printf("client:\tsend to: %d, delta = %d, rank = %d\n", 0, delta, rank);
        rc = MPI_Recv(&buf, 2, MPI_INT, 0, 1, MPI_COMM_WORLD, &status);
        check(rc);
        printf("received from: %d, delta = %d, rank = %d\n", 0, buf, rank);
        local_time += buf;
        printf("new_time = %d, rank = %d\n", local_time, rank);
    }
    MPI_Finalize();
    return 0;
}
