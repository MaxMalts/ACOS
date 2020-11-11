#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

enum StdFds { STDIN = 0, STDOUT = 1, STDERR = 2 };

int ProcessCommand(const char* command, const int fin, const int fout)
{
    assert(command != NULL);
    assert(fin >= 0);
    assert(fout >= 0);

    pid_t process = fork();
    if (-1 == process)
        return -1;
    if (0 == process) {
        if (STDIN != fin) {
            dup2(fin, STDIN);
            close(fin);
        }
        if (STDOUT != fout) {
            dup2(fout, STDOUT);
            close(fout);
        }

        execlp(command, command, NULL);
        exit(-1);
    }

    return process;
}

int main(const int argc, const char* argv[])
{
    if (argc == 1)
        return 0;

    if (argc == 2) {
        execlp(argv[1], argv[1], NULL);
        return -1;
    }

    int pipe1[2] = {0};
    int pipe2[2] = {0};

    int* curInPipe = pipe1;
    int* curOutPipe = pipe2;
    for (int i = 1; i < argc - 1; ++i) {
        pipe(curOutPipe);

        ProcessCommand(argv[i], curInPipe[0], curOutPipe[1]);
        close(curOutPipe[1]);

        int status = 0;
        wait(&status);

        close(curInPipe[0]);
        close(curInPipe[1]);

        int* tempPipe = curInPipe;
        curInPipe = curOutPipe;
        curOutPipe = tempPipe;
    }

    ProcessCommand(argv[argc - 1], curInPipe[0], STDOUT);
    close(curInPipe[0]);
    close(curInPipe[1]);

    int status = 0;
    wait(&status);

    return 0;
};