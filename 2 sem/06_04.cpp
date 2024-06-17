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
    
    int times[size];
    for (int i = 0; i < size; ++i)
        times[i] = 0;

    std::string filename{std::to_string(rank + 1)};
    if (rank + 1 < 10)
        filename = "0" + filename + ".dat";
    else
        filename = filename + ".dat";
    filename = "v" + filename;
    // printf("----%d %s\n", rank, filename.c_str());
    
    int k = 0, d = 1, local_time = 0;
    FILE *file = std::fopen(filename.c_str(), "r");
    if (file != nullptr)
    {
        while (std::fscanf(file, "%d", &k) != EOF)
        {
            times[rank] += d;
            if (k == 0)
            {
                printf("rank: %d\t internal event \t time:\t[ ", rank);
                for (int i = 0; i < size; ++i)
                    printf("%d, ", times[i]);
                printf("]\n");
            }
            else if (k > 0)
            {
                printf("rank: %d\t send event to %d\t time:\t[ ", rank, k);
                for (int i = 0; i < size; ++i)
                    printf("%d, ", times[i]);
                printf("]\n");
                rc = MPI_Send(times, size, MPI_INT, k - 1, rank + 1, MPI_COMM_WORLD);
                check(rc);
            }
            else
            {
                int buf_times[size];
                rc = MPI_Recv(buf_times, size, MPI_INT, -k - 1, -k, MPI_COMM_WORLD, &status);
                check(rc);
                for (int i = 0; i < size; ++i)
                {
                    if (i != rank)
                        times[i] = std::max(times[i], buf_times[i]);
                }
                printf("rank: %d\t recv event from %d\t time:\t[ ", rank, -k);
                for (int i = 0; i < size; ++i)
                    printf("%d, ", times[i]);
                printf("]\n");
            }
        }
        std::fclose(file);
    }
    else
    {
        std::cout << "Can't open file or no file: " << filename << std::endl;
    }
    MPI_Finalize();
    return 0;
}
