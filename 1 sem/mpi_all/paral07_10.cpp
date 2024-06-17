#include <iostream>
#include <cmath>
#include <iomanip>
#include <fstream>
#include <string>
#include <mpi.h>

/*
broadcast - не принимать с помощью recv
*/

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


int main(int argc, char* argv[])
{
    int N = 0;
    int rank, size, count;
    int buf[3];
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
		std::ifstream data;
		data.open("pi_broadcast.dat", std::ios::in);
        data >> buf[2];
		data.close();
	}
    N = buf[2];
    int broadcast = MPI_Bcast(&buf[2], 1, MPI_INT, 0, MPI_COMM_WORLD);
    check(broadcast);
    MPI_Get_count(&status, MPI_INT, &count);
    buf[0] = buf[2] / size * rank;
    buf[1] = buf[2] / size * (rank + 1);
    double s = area(buf[0], buf[1], buf[2]);
    int reduce = MPI_Reduce(&s, &pi, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD); 
    check(reduce);
    MPI_Finalize();
    // std::cout << "Pi: " << std::cout.precision(14) << pi << std::endl;
    if (rank == 0)
    {
        std::cout << "Pi: " << pi << std::endl;
    }
    return 0;
}