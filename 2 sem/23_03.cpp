#include <mpi.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>

void check(int rc)
{
    if (rc != MPI_SUCCESS)
    {
        std::cout << "MPI eroor: " << rc;
        MPI_Abort(MPI_COMM_WORLD, rc);
    }
}

int main(int argc, char *argv[])
{
    int rank, size, count;
    int rc;
    rc = MPI_Init(&argc, &argv);

    rc = MPI_Comm_size(MPI_COMM_WORLD, &size);
    check(rc);
    rc = MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    check(rc);
    MPI_Status status;

    std::string filename{std::to_string(rank + 1)};
    if (rank + 1 < 10)
        filename = "0" + filename + ".dat";
    else
        filename = filename + ".dat";
    // printf("----%d %s\n", rank, filename.c_str());
    
    int k = 0, d = 1, local_time = 0;
    FILE *file = std::fopen(filename.c_str(), "r");
    if (file != nullptr)
    {
        while (std::fscanf(file, "%d", &k) != EOF)
        {
            local_time += d;
            if (k == 0)
                printf("rank: %d\t internal event \t time: %d\n", rank, local_time);
            else if (k > 0)
            {
                printf("rank: %d\t send event to %d\t time: %d\n", rank, k, local_time);
                rc = MPI_Send(&local_time, 1, MPI_INT, k - 1, rank + 1, MPI_COMM_WORLD);
                check(rc);
            }
            else
            {
                int buf_time;
                rc = MPI_Recv(&buf_time, 1, MPI_INT, -k - 1, -k, MPI_COMM_WORLD, &status);
                check(rc);
                local_time = std::max(local_time, buf_time + d);
                printf("rank: %d\t recv event from %d\t time: %d\n", rank, -k, local_time);
            }
        }
        std::fclose(file);
    }
    else
    {
        std::cerr << "Can't open file or no file: " << filename << std::endl;
    }
    MPI_Finalize();
    return 0;
}
