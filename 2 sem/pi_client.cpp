
/* A simple echo UDP client */

#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    int sockfd, n, len, err;
    char sendline[1000], recvline[1000];
    struct sockaddr_in servaddr, cliaddr;
    unsigned short port;

    if (argc < 2 || argc > 3)
    {
        printf("Usage: a.out <IP address> <port - default 51000>\n");
        exit(1);
    }

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;

    if (argc == 3)
    {
        err = sscanf(argv[2], "%d", &port);
        if (err != 1 || port == 0)
        {
            printf("Invalid port\n");
            exit(-1);
        }
    }
    else
    {
        port = 51000;
    }

    printf("Port set to %d\n", port);

    servaddr.sin_port = htons(port);

    if (inet_aton(argv[1], &servaddr.sin_addr) == 0)
    {
        printf("Invalid IP address\n");
        exit(-1);
    }

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        printf("Can\'t create socket, errno = %d\n", errno);
        exit(1);
    }

    bzero(&cliaddr, sizeof(cliaddr));
    cliaddr.sin_family = AF_INET;
    cliaddr.sin_port = htons(0);
    cliaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sockfd, (struct sockaddr *)&cliaddr, sizeof(cliaddr)) < 0)
    {
        printf("Can\'t bind socket, errno = %d\n", errno);
        close(sockfd);
        exit(1);
    }

    int N, K;
    printf("N => ");
    scanf("%d", &N);
    printf("K => ");
    scanf("%d", &K);

    int step = N / K;
    int ibeg = 0;
    int iend = step;

    double ans = 0;

    for (int i = 0; i < K; i++)
    {
        ibeg = i * step;
        iend = (i + 1) * step;
        if (i == K - 1)
            iend = N;

        int send_arr[3];
        send_arr[0] = N;
        send_arr[1] = ibeg;
        send_arr[2] = iend - 1;

        printf("%d: n: %d\t ibeg: %d\t iend: %d\n", i, send_arr[0], send_arr[1], send_arr[2]);

        if (sendto(sockfd, send_arr, 3 * sizeof(int), 0, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
        {
            printf("Can\'t send request, errno = %d\n", errno);
            close(sockfd);
            exit(1);
        }
    }

    for (int i = 0; i < K; i++)
    {
        double recv = 0;
        if ((n = recvfrom(sockfd, &recv, sizeof(double), 0, (struct sockaddr *)NULL, NULL)) < 0)
        {
            printf("Can\'t receive answer, errno = %d\n", errno);
            close(sockfd);
            exit(1);
        }
        ans += recv;
    }

    printf("\nPI = %f\n", ans);

    close(sockfd);
    return 0;
}
