#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(const int argc, const char* argv[])
{
    if (argc != 3)
        return -1;

    int pipefd[2] = {0};
    if (pipe(pipefd) == -1)
        return -1;

    pid_t process1 = fork();
    if (-1 == process1) {
        return -1;
    } else if (0 == process1) {
        if (dup2(pipefd[1], 1) == -1)
            return -1;

        close(pipefd[0]);
        close(pipefd[1]);

        execlp(argv[1], argv[1], NULL);
        return -1;
    }

    pid_t process2 = fork();
    if (-1 == process2) {
        return -1;
    } else if (0 == process2) {
        if (dup2(pipefd[0], 0) == -1)
            return -1;

        close(pipefd[0]);
        close(pipefd[1]);

        execlp(argv[2], argv[2], NULL);
        return -1;
    }

    close(pipefd[0]);
    close(pipefd[1]);

    for (int i = 0; i < 2; ++i) {
        int exitStatus = 0;
        wait(&exitStatus);
        if (!WIFEXITED(exitStatus) || WEXITSTATUS(exitStatus) != 0) {
            return -1;
        }
    }

    return 0;
}