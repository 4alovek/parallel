#include <iostream>
#include <string>
#include <cmath>
#include <omp.h>

const char* file_with_n = "N.dat";

int read_n(){
    FILE* file = fopen(file_with_n, "r");
    int n = 0;
    fscanf(file, "%d", &n);
    fclose(file);
    return n;
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

int main(){
    int N = read_n();
    std::cout << "N = " << N << std::endl;

    int size = omp_get_num_procs();
    double s = 0;

#pragma omp parallel num_threads(size) reduction(+:s)
    {
        int rank = omp_get_thread_num();

        int area_start = 0, area_stop = 0;
        if (rank == size-1){
            area_start = (size-1)*N/size;
            area_stop = N-1;
        } else {
            area_start = rank*N/size;
            area_stop = (rank+1)*N/size; 
        }

        double part_ans = area(area_start, area_stop, N);
        std::cout << "rank = " << rank << " part sum = " << part_ans << std::endl;
        s += part_ans;
    }

    printf("PI = %f\n", s);
}