#define _DEFAULT_SOURCE

#include <arpa/inet.h>
#include <assert.h>
#include <netinet/ip.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int main(const int argc, const char* argv[])
{
    if (argc != 2) {
        return -1;
    }

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (-1 == sockfd) {
        return -1;
    }

    struct sockaddr_in sockAddr = {};
    sockAddr.sin_family = AF_INET;
    uint16_t port = strtol(argv[1], NULL, 10);
    sockAddr.sin_port = htons(port);
    sockAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(
            sockfd, (const struct sockaddr*)(&sockAddr), sizeof(sockAddr)) ==
        -1) {
        return -1;
    }

    int curNum = 0;
    while (scanf("%d", &curNum) > 0) {
        if (sendto(
                sockfd,
                &curNum,
                sizeof(curNum),
                0,
                (const struct sockaddr*)(&sockAddr),
                sizeof(sockAddr)) == -1) {
            close(sockfd);
            return -1;
        }

        ssize_t bytesRead = recv(sockfd, &curNum, sizeof(curNum), 0);

        if (-1 == bytesRead) {
            close(sockfd);
            return -1;
        }

        printf("%d\n", curNum);
    }

    close(sockfd);

    return 0;
}