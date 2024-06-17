#include <mpi.h>
#include <stdio.h>
#include <math.h>


double func(double x){
    if (x*x >= 4)
        return 0;
    return sqrt(4-x*x);
}


double integral(double l, double r, int n){
    if (n == 0) return 0;

    double step = (r-l)/n;
    double x = l;
    double ans = 0;

    for (int i=0; i<n; ++i){
        ans += step * (func(x) + func(x+step)) * 0.5;
        x += step;
    }

    return ans;
}


void check(int val){
    if (val != MPI_SUCCESS){
        printf("ERROR\n");
        MPI_Abort(MPI_COMM_WORLD, val);
    }
}



int main(int argc, char* argv[]){
    double L = 0;
    double R = 2;
    int N = 40;

    int init_val = MPI_Init(&argc, &argv);
    check(init_val);

    int rank=0;
    int rank_val = MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    check(rank_val);

    // [l, r, n]
    double buf[3];
    MPI_Status status;

    if (rank == 0) {	
        printf("\nN = %d\n\n", N);
    
        int size = 0;
        int size_val = MPI_Comm_size(MPI_COMM_WORLD, &size);
        check(size_val);
        size--;

        int n = N / size;
        int count_plus1 = N % size; 
        double step = (R-L)/N;
        double last_r = L;
        for (int proc_numb=1; proc_numb<=size; ++proc_numb){
            buf[2] = n + (count_plus1 > 0);
            --count_plus1;

            buf[0] = last_r;
            buf[1] = last_r + step * buf[2];

            last_r = buf[1];

            int send_val = MPI_Send(buf, 4, MPI_DOUBLE, proc_numb, 99, MPI_COMM_WORLD);
            check(send_val);
            printf("rank 0: send to %d, L = %f, R = %f, N = %d\n", 
                    proc_numb, buf[0], buf[1], (int)buf[2]);
        }

        double answer = 0;
        double proc_ans = 0;
        for (int proc_numb=1; proc_numb<=size; ++proc_numb){
            int recv_val = MPI_Recv(&proc_ans, 1, MPI_DOUBLE, 
                    MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            check(recv_val);

            answer += proc_ans;
        }

        printf("\nPI = %f\n\n", answer);
    } else {
        int recv_val = MPI_Recv(buf, 4, MPI_DOUBLE, 0, 
                MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        check(recv_val);

        rank_val = MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        check(rank_val);

        printf("rank %d: received L = %f, R = %f, N = %d\n",
                rank, buf[0], buf[1], (int)buf[2]);

        double ans = integral(buf[0], buf[1], (int)buf[2]);

        int send_val = MPI_Send(&ans, 1, MPI_DOUBLE, 0, 99, MPI_COMM_WORLD);
        check(send_val);

        printf("rank %d: send to 0: %f\n", rank, ans); 
    }

    MPI_Finalize();

    return 0;
}
