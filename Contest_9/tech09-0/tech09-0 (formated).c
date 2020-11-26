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
    if (argc != 3) {
        return -1;
    }

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == sockfd) {
        return -1;
    }

    struct sockaddr_in sockAddr = {};
    sockAddr.sin_family = AF_INET;
    uint16_t port = strtol(argv[2], NULL, 10);
    sockAddr.sin_port = htons(port);
    sockAddr.sin_addr.s_addr = inet_addr(argv[1]);

    if (connect(
            sockfd, (const struct sockaddr*)(&sockAddr), sizeof(sockAddr)) ==
        -1) {
        return -1;
    }

    int curNum = 0;
    while (scanf("%d", &curNum) > 0) {
        if (write(sockfd, &curNum, sizeof(curNum)) == -1) {
            close(sockfd);
            return 0;
        }

        int bytesRead = read(sockfd, &curNum, sizeof(curNum));

        if (-1 == bytesRead) {
            close(sockfd);
            return 0;
        } else if (0 == bytesRead) {
            shutdown(sockfd, SHUT_RDWR);
            close(sockfd);
            return 0;
        }

        printf("%d\n", curNum);
    }

    close(sockfd);

    return 0;
}