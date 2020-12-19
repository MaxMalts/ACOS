#define _XOPEN_SOURCE 700

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <unistd.h>

int needToTerm = 0;

void TermHandler(int signal)
{
    needToTerm = 1;
}

int CreateListenSock(const char* ipAddr, const int port)
{
    int listenSock = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == listenSock) {
        return -1;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ipAddr);
    addr.sin_port = htons(port);

    if (-1 == bind(listenSock, (const struct sockaddr*)&addr, sizeof(addr))) {
        close(listenSock);
        return -1;
    }

    if (-1 == listen(listenSock, SOMAXCONN)) {
        close(listenSock);
        return -1;
    }
    
    return listenSock;
}

void AcceptConnections(int listenSock, int epoll)
{
    int clientSock = accept(listenSock, NULL, NULL);
    while (clientSock != -1) {
        int flags = fcntl(clientSock, F_GETFL);
        flags |= O_NONBLOCK;
        fcntl(clientSock, F_SETFL, flags);

        struct epoll_event pollEvent = { .events = EPOLLIN, .data.fd = clientSock };
        epoll_ctl(epoll, EPOLL_CTL_ADD, clientSock, &pollEvent);

        clientSock = accept(listenSock, NULL, NULL);
    }
}

int ProccesConnection(int clientSock)
{
    char buf[10000] = "";

    int bytesRead = read(clientSock, buf, sizeof(buf));
    if (-1 == bytesRead) {
        shutdown(clientSock, SHUT_RDWR);
        close(clientSock);
        return -1;
    }

    for (int i = 0; i < bytesRead; ++i) {
        buf[i] = toupper(buf[i]);
    }

    int bytesWrote = write(clientSock, buf, bytesRead);
    if (-1 == bytesWrote) {
        shutdown(clientSock, SHUT_RDWR);
        close(clientSock);
    }

    return 0;
}

int main(const int argc, const char* argv[])
{
    if (argc != 2) {
        return -1;
    }

    struct sigaction action = { .sa_handler = TermHandler, .sa_flags = SA_RESTART };
    sigaction(SIGTERM, &action, NULL);

    int listenSock = CreateListenSock("127.0.0.1", atoi(argv[1]));
    int flags = fcntl(listenSock, F_GETFL);
    flags |= O_NONBLOCK;
    fcntl(listenSock, F_SETFL, flags);

    int epoll = epoll_create1(0);
    if (-1 == epoll) {
        return -1;
    }
    struct epoll_event pollEvent = { .events = EPOLLIN, .data.fd = listenSock };
    epoll_ctl(epoll, EPOLL_CTL_ADD, listenSock, &pollEvent);

    struct epoll_event curEvents[10000];
    while (!needToTerm) {
        int nEvents = epoll_wait(epoll, curEvents, sizeof(curEvents) / sizeof(curEvents[0]), -1);
        if (-1 == nEvents) {
            close(epoll);
            close(listenSock);
            return -1;
        }

        for (int i = 0; i < nEvents; ++i) {
            int curSock = curEvents[i].data.fd;
            if (curSock == listenSock) {
                AcceptConnections(listenSock, epoll);
            } else {
                ProccesConnection(curSock);
            }
        }
    }

    close(epoll);
    close(listenSock);
    return 0;
}

