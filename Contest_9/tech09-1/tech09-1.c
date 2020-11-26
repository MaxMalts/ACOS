#define _XOPEN_SOURCE 700
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
 
#include <sys/socket.h>
#include <sys/types.h>
 
#include <netinet/in.h>
 
const size_t BUF_LEN = 4096;
char waiting = 0;
sig_atomic_t IS_RUNNING = 1;
void sigterm_handler(int sig_num)
{
    IS_RUNNING = 0;
    if (waiting)
        exit(0);
}
 
void make_sigaction(struct sigaction* sigact, void (*func)(int), int flags)
{
    memset(sigact, 0, sizeof(struct sigaction));
    sigact->sa_flags = flags;
    sigact->sa_handler = func;
}
 
int main(int argc, char* argv[])
{
    printf("%d", getpid());
 
    if (argc != 3)
        return 1;
 
    struct sigaction sigterm;
    make_sigaction(&sigterm, sigterm_handler, SA_RESTART);
    if (sigaction(SIGTERM, &sigterm, NULL) == -1)
        return 1;
    if (sigaction(SIGINT, &sigterm, NULL) == -1)
        return 1;
 
    char ip_str[] = "127.0.0.1";
    unsigned int ip = 0;
    for (int i = 0; i < sizeof(ip); ++i) {
        *((char*)&ip + i) = atoi(strtok(i == 0 ? ip_str : NULL, "."));
    }
 
    struct in_addr ip_struct;
    ip_struct.s_addr = 0;
 
    struct sockaddr_in sockaddr;
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = htons(atoi(argv[1]));
    ;
    sockaddr.sin_addr = ip_struct;
 
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
 
    if (bind(socket_fd, (struct sockaddr*)&sockaddr, sizeof(sockaddr)) == -1)
        return 1;
 
    if (listen(socket_fd, SOMAXCONN) == -1)
        return 1;
 
    char buff[BUF_LEN];
    char junk[BUF_LEN];
 
    while (IS_RUNNING) {
        waiting = 1;
        int client_fd = accept(socket_fd, 0, 0);
        if (client_fd == -1)
            return 1;
        waiting = 0;
 
        int bytes = 0;
        while (bytes < BUF_LEN &&
               (bytes = read(client_fd + bytes, buff, BUF_LEN - 1)) > 0)
            if (strstr(buff, "\r\n\r\n") != 0)
                break;
 
        char path[BUF_LEN];
        //if (*argv[2] != '/')
        strncpy(path, argv[2], BUF_LEN);
        //else
        //strncpy(path, argv[2] + 1, BUF_LEN);
 
        int len = strlen(path);
        if (path[len - 1] != '/' && len < BUF_LEN) {
            path[len] = '/';
            ++len;
        }
        char* filename = buff + 4;
 
        char* filename_end = strstr(filename, " HTTP/1.1");
        *filename_end = 0;
 
        strcat(path, filename);
        printf("%s\n", path);
        struct stat file_stat;
        if (lstat(path, &file_stat) == -1) {
            dprintf(client_fd, "HTTP/1.1 404 Not Found\r\n");
        } else if (access(path, R_OK) == -1) {
            dprintf(client_fd, "HTTP/1.1 403 Forbidden\r\n");
        } else {
            char outbuff[BUF_LEN * 16];
            snprintf(
                outbuff,
                BUF_LEN * 16 - 1,
                "HTTP/1.1 200 OK\r\nContent-Length: %ld\r\n\r\n",
                file_stat.st_size);
            char* start_write = outbuff + strlen(outbuff);
 
            int fd = open(path, O_RDONLY);
            read(fd, start_write, outbuff + BUF_LEN * 16 - start_write);
 
            write(client_fd, outbuff, strlen(outbuff));
        }
 
        close(client_fd);
        shutdown(client_fd, SHUT_RDWR);
    }
    close(socket_fd);
    shutdown(socket_fd, SHUT_RDWR);
}