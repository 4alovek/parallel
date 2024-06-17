#include <iostream>
#include <stdio.h>
#include <algorithm>
#include <stdlib.h>
#include <cmath>
#include <string>
#include <iomanip>
#include "omp.h"

int main(int argc, char *argv[])
{
    int N;
    int size, count;
    if (argc != 2)
    {
        printf("Usage: exefile npoints\n");
        return -1;
    }
    
    double delta, maxdelta;
    double eps=1.e-6;
    double h, tau;
    FILE *ff;
    int i;
    N = atoi(argv[1]);
    if (N == 0) {
        std::cout << "Set N to 1000" << std::endl;
        N = 1000;
    }
    else {
        std::cout << "Set N to " << N << std::endl;
    }
    double* u = new double[N + 1];
    double* unew = new double[N + 1];

    for(i = 1; i < N; i++)
        u[i] = 0;
        
    unew[0] = u[0] = 1.0;
    unew[N] = u[N] = 0;

    h = 1.0/N;
    tau = 0.5*(h*h);

    size = omp_get_num_procs();

while (1) {
    double maxx[size];
    for(i = 0; i < size; ++i)
        maxx[i] = 0;

#pragma omp parallel
    {
#pragma omp for 
        for (int i=1; i<N; i++)
            unew[i] = u[i]+(tau/(h*h))*(u[i-1]-2*u[i]+u[i+1]);
            
#pragma omp for private(delta)
        for (int i=1; i<N; i++){
            delta = fabs(unew[i] - u[i]);
            int j = omp_get_thread_num();
            if (maxx[j] > delta) continue;
            else maxx[j] = delta;
        }
    }

    double max = *std::max_element(maxx, maxx+size);
    if (max < eps) break;


#pragma omp parallel for 
        for (int i=1; i<N; i++)
            u[i] = unew[i];

}
    freopen("seqres_omp", "w", stdout);
    std::cout << std::fixed << std::setprecision(6);
    for (int i=0; i<=N; i++){
        std::cout << unew[i] << '\n';
    }
    fclose(stdout);

    return 0;
}
