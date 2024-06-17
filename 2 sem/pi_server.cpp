#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <cmath>

double func(double x) {
    if (x >= 2) return 0;
    return sqrt(4-x*x);
}

double area(int N, int ibeg, int iend) {
    double step = 2.0 / N;
    double ans = 0;

    for (int i = ibeg; i <= iend; i++) {
        double x = i * step;
        ans += step * (func(x) + func(x+step)) * 0.5;
    }

    return ans;
}


int main()
{
    int sockfd, n;
    int recv_arr[3];
    unsigned clilen;
    double s;
    struct sockaddr_in servaddr, cliaddr;

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(52031);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        printf("Can\'t create socket, errno = %d\n", errno);
        exit(1);
    }

    if (bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        printf("Can\'t bind socket, errno = %d\n", errno);
        close(sockfd);
        exit(1);
    }

    while (1)
    {
        clilen = sizeof(cliaddr);
        if ((n = recvfrom(sockfd, recv_arr, 3 * sizeof(int), 0, (struct sockaddr *)&cliaddr, &clilen)) < 0)
        {
            printf("Can\'t receive request, errno = %d\n", errno);
            close(sockfd);
            exit(1);
        }
        s = area(recv_arr[0], recv_arr[1], recv_arr[2]);

        // printf("from %s %f\n", inet_ntoa(cliaddr.sin_addr), s);
        if (sendto(sockfd, &s, sizeof(double), 0, (struct sockaddr *)&cliaddr, clilen) < 0)
        {
            printf("Can\'t send answer, errno = %d\n", errno);
            close(sockfd);
            exit(1);
        }
    }
    return 0;
}
