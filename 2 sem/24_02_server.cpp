
/* A simple echo UDP server */

#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>

double func(double x) {
    if (x >= 2) return 0;
    return sqrt(4-x*x);
}

double calculate(int N, int ib, int ie) {
    double L = 0, R = 2;
    double step = (R - L) / N;
    double ans = 0;

    for (int i = ib; i <= ie; i++) {
        double x = i * step;
        ans += step * (func(x) + func(x+step)) * 0.5;
    }

    return ans;
}


int main() {
    unsigned int clilen;
    int  sockfd, n;
    int  recv[3];
    struct sockaddr_in servaddr, cliaddr;
    
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family      = AF_INET;
    servaddr.sin_port        = htons(52031);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
       printf("Can\'t create socket, errno = %d\n", errno);
       exit(1);
    }

    if(bind(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0){
       printf("Can\'t bind socket, errno = %d\n", errno);
       close(sockfd);
       exit(1);
    }

    while (1) {
        clilen = sizeof(cliaddr);
        
        if((n = recvfrom(sockfd, recv, 3*sizeof(int), 0, (struct sockaddr *) &cliaddr, &clilen)) < 0){
            printf("Can\'t receive request, errno = %d\n", errno);
            close(sockfd);
            exit(1);
        }

        printf("receive: %d, %d, %d\n", recv[0], recv[1], recv[2]);

        double ans = calculate(recv[0], recv[1], recv[2]);
        
        printf("calculate: %f\n", ans);

        if(sendto(sockfd, &ans, sizeof(double), 0, (struct sockaddr *) &cliaddr, clilen) < 0){
            printf("Can\'t send answer, errno = %d\n", errno);
            close(sockfd);
            exit(1);
        }

        printf("send\n");
    }


    return 0;
}
