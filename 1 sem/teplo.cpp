#include <iostream>
#include <fstream>
#include <string>
#include <cmath>
#include <stdlib.h>
#include <mpi.h>

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
    MPI_Status status;
    rc = MPI_Init(&argc, &argv);
    check(rc);
    rc = MPI_Comm_size (MPI_COMM_WORLD, &size);
    check(rc);
    rc = MPI_Comm_rank (MPI_COMM_WORLD, &rank);
    check(rc);
    rc = MPI_Get_count(&status, MPI_INT, &count);
    check(rc);
    if (argc != 2)
    {
        printf("Usage: exefile npoints\n");
	    MPI_Abort(MPI_COMM_WORLD, -1);
        return -1;
    }

    double delta, maxdelta;
    double eps=1.e-6;
    double h, tau;
    int N;
    FILE *ff;
    int i;
    N = atoi(argv[1]);
    if (N == 0) {
        if (rank == 0)
            std::cout << "Set N to 1000" << std::endl;
        N = 1000;
    }
    else {
        if (rank == 0)
            std::cout << "Set N to " << N << std::endl;
    }
    double* u = new double[N + 1];
    double* unew = new double[N + 1];

    // begin & bound values
    int jbeg = rank * (N - 1) / size + 1;
    int jend = (rank + 1) * (N - 1) / size;
    if (rank == size - 1)
        jend = N - 1;

    for(i = 1; i < N; i++)
        u[i] = 0;
        
    unew[0] = u[0] = 1.0;
    unew[N] = u[N] = 0;
    
    h = 1.0/N;
    tau = 0.5*(h*h);

    while(1)
    {
	    for (i = jbeg; i <= jend; i++)
	        unew[i] = u[i]+(tau/(h*h))*(u[i-1]-2*u[i]+u[i+1]);

        maxdelta = 0;
        for (i = jbeg; i < jend; i++)
        {
            delta = fabs(unew[i]-u[i]);
            if (delta > maxdelta) maxdelta = delta;
        }

        double general_maxdelta = 0;
        rc = MPI_Reduce(&maxdelta, &general_maxdelta, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
        check(rc);
        rc = MPI_Bcast(&general_maxdelta, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
        check(rc);
        if(general_maxdelta < eps) break;
        if (rank > 0){
            rc = MPI_Send(&unew[jbeg], 1, MPI_DOUBLE, rank-1, 99, MPI_COMM_WORLD);
            check(rc);
        }
        if (rank < size-1){
            rc = MPI_Send(&unew[jend], 1, MPI_DOUBLE, rank+1, 99, MPI_COMM_WORLD);
            check(rc);
        }
        if (rank > 0){
            rc = MPI_Recv(&unew[jbeg-1], 1, MPI_DOUBLE, rank-1, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            check(rc);
        }
        if (rank < size-1){
            rc = MPI_Recv(&unew[jend+1], 1, MPI_DOUBLE, rank+1, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            check(rc);
        }
        for(i = jbeg - 1; i <= jend + 1; i++)
    	    u[i] = unew[i];   
    }
    // std::cout << "Im alive";

    int recvcounts[size];
    int displs[size];
    int gat_size = jend - jbeg + 1;
    rc = MPI_Gather(&gat_size, 1, MPI_INT, recvcounts, 1, MPI_INT, 0, MPI_COMM_WORLD);
    check(rc);  
    if (rank == 0)
    {
        for (int i = 0; i < size; ++i)
        {
            if (i == 0)
                displs[i] = 1;
            else
                displs[i] = displs[i - 1] + recvcounts[i - 1];
        }
    }
    double* ures = new double[N + 1]; 
    ures[0] = 1.;
    ures[N] = 0.;
    rc = MPI_Gatherv(&unew[jbeg], gat_size, MPI_DOUBLE, ures, recvcounts, displs, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    check(rc);
 
    MPI_Finalize();
    // if (rank == 0)
	// {
	// 	std::fstream data;
	// 	data.open("seqres_mpi", std::ios::out);
	// 	for (int i = 0; i < N + 1; ++i)
    //     {
    //         data << ures[i] << '\n';
    //     }
    //     data.close();
	// }
    if (rank == 0)
    {
        if ((ff = fopen("seqres_mpi", "w+")) == NULL){
        printf("Can't open file\n");
        exit(-1);
        }
        for(i=0; i<N+1; i++)
        fprintf(ff, "%f\n", ures[i]);
        fclose(ff);
    }
    return 0; 
}      