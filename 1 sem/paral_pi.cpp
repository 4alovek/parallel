#include <iostream>
#include <string>
#include <mpi.h>
#include <cmath>


void check(int rc) {
    if (rc != MPI_SUCCESS){
    std::cout << "MPI eroor: " << rc;
    MPI_Abort(MPI_COMM_WORLD, rc);
    }
}


double area(int area_start, int area_stop, int N)
{
    double s = 0;
    double height1 = 0;
    double height2 = 0;
    for (int i = area_start; i < area_stop; ++i)
    {
        height1 = sqrt(4.0 - pow((i*(2.0/N)), 2));
        height2 = sqrt(4.0 - pow(((i+1)*(2.0/N)), 2));
        s = s + (height1 + height2)/2* (2.0 / N);
    }
    return s;
}


int main (int argc, char* argv[])
{
    // int N = 1000000000;
    int N = 100000;
    int rank, size, count;
    int buf[3];
    buf[2] = N;
    int rc;
    double pi = 0;
    MPI_Status status;
    rc = MPI_Init(&argc, &argv);
    check(rc);
    rc = MPI_Comm_size (MPI_COMM_WORLD, &size);
    check(rc);
    rc = MPI_Comm_rank (MPI_COMM_WORLD, &rank);
    check(rc);
    if (rank == 0)
    {
        for (int i = 0; i < size; ++i)
        {
            buf[0] = N / size * i;
            buf[1] = N / size * (i + 1);
            std::cout << "Start " << buf[0] << " Stop " << buf[1] << " to " << i << std::endl;
            int send_message = MPI_Send(buf, 3, MPI_INT, i, 0, MPI_COMM_WORLD);
            check(send_message);
        }
    }       
    int receive_message = MPI_Recv(buf, 3, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
    check(receive_message);
    MPI_Get_count(&status, MPI_INT, &count);

    double s = area(buf[0], buf[1], buf[2]);
    if (rank != 0)
    {
        int send_message = MPI_Send(&s, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
        check(send_message);
        std::cout << "area " << s << " rank " << rank << std::endl;
    }
    else
    {
        pi += s;
        for (int i = 0; i < size - 1; ++i)
        {
            int receive_message = MPI_Recv(&s, 1, MPI_DOUBLE, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
            check(receive_message);
            std::cout << "received area: " << s << " from " << status.MPI_SOURCE << std::endl;
            pi += s;
        }
        // std::cout << "Pi: " << std::cout.precision(14) << pi << std::endl;
        std::cout << "Pi: " << pi << std::endl;
    }
    MPI_Finalize();
    return 0;
}