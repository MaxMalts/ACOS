#define _XOPEN_SOURCE 700

#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/limits.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int needToFinish = 0;

void Finish(int signal)
{
    needToFinish = 1;
}

int AddSigactions()
{
    struct sigaction sigterm = {};
    sigterm.sa_flags = SA_RESTART;
    sigterm.sa_handler = Finish;

    if (sigaction(SIGINT, &sigterm, NULL) == -1 ||
        sigaction(SIGTERM, &sigterm, NULL) == -1) {

        return -1;
    }

    return 0;
}

int InitServer(const char* ip_str, int port)
{
    assert(ip_str != NULL);
    assert(port > 0);

    struct sockaddr_in sockAddr;
    sockAddr.sin_family = AF_INET;
    sockAddr.sin_addr.s_addr = inet_addr(ip_str);
    sockAddr.sin_port = htons(port);

    int resSock = socket(AF_INET, SOCK_STREAM, 0);

    if (bind(resSock, (struct sockaddr*)&sockAddr, sizeof(sockAddr)) == -1) {
        return -1;
    }

    if (listen(resSock, SOMAXCONN) == -1) {
        return -1;
    }

    return resSock;
}

void HandleClient(int clientSock, const char* dataDir)
{
    assert(clientSock > 0);

    const int bufLen = 20000;
    char buf[bufLen];
    char fullPath[PATH_MAX] = "";

    int bytesRead = read(clientSock, buf, bufLen - 1);
    buf[bytesRead] = '\0';

    char garbage[bufLen];
    garbage[0] = '\0';
    while (strstr(garbage, "\r\n\r\n") != 0) {
        bytesRead = read(clientSock, garbage, sizeof(garbage) - 1);
        garbage[bytesRead] = '\0';
    }

    char* reqPath = buf + 4;
    *(strstr(reqPath, " HTTP/1.1")) = '\0';

    strncpy(fullPath, dataDir, PATH_MAX);
    strcat(fullPath, reqPath);

    struct stat fStats;
    if (lstat(fullPath, &fStats) == -1) {
        char writeStr[] = "HTTP/1.1 404 Not Found\r\n";
        write(clientSock, writeStr, sizeof(writeStr) - 1);

    } else if (access(fullPath, R_OK) == -1) {
        char writeStr[] = "HTTP/1.1 403 Forbidden\r\n";
        write(clientSock, writeStr, sizeof(writeStr) - 1);

    } else {
        lstat(fullPath, &fStats);
        snprintf(
            buf,
            bufLen - 1,
            "HTTP/1.1 200 OK\r\nContent-Length: %ld\r\n\r\n",
            fStats.st_size);
        int headerLen = strlen(buf);

        int file = open(fullPath, O_RDONLY);
        int bytesRead = read(file, buf + headerLen, bufLen - headerLen - 1);
        assert(bytesRead < bufLen - headerLen - 1);
        close(file);

        write(clientSock, buf, headerLen + bytesRead);
    }

    shutdown(clientSock, SHUT_RDWR);
    close(clientSock);
}

int RunServer(int listenSock, const char* dataDir)
{
    printf("%s\n", dataDir);
    assert(listenSock > 0);

    int listenSockFlags =
        fcntl(listenSock, F_GETFL); // For needToFinish to work
    listenSockFlags |= O_NONBLOCK;
    fcntl(listenSock, F_SETFL, listenSockFlags);

    while (1) {
        int clientSock = 0;
        do {
            if (needToFinish) {
                return 0;
            }
            clientSock = accept(listenSock, 0, 0);
        } while (-1 == clientSock);

        HandleClient(clientSock, dataDir);
    }
}

int main(const int argc, const char* argv[])
{
    if (argc != 3) {
        return -1;
    }

    if (AddSigactions() == -1) {
        return -1;
    }

    int listenSock = InitServer("127.0.0.1", atoi(argv[1]));
    if (-1 == listenSock) {
        return -1;
    }

    char dataDir[strlen(argv[2]) + 2];
    strncpy(dataDir, argv[2], sizeof(dataDir) - 1);
    int dataDirLen = strlen(dataDir);
    if (dataDir[dataDirLen - 1] != '/') {
        dataDir[dataDirLen] = '/';
        dataDir[dataDirLen + 1] = '\0';
    }
    RunServer(listenSock, dataDir);

    shutdown(listenSock, SHUT_RDWR);
    close(listenSock);
    return 0;
}