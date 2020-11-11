#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdint.h>
#include <assert.h>



int StartTask(uint64_t N) {
    assert(N > 0);

    uint64_t curNum = 1;
    while (curNum < N) {
        printf("%lu ", curNum);
        fflush(stdout);
        ++curNum;

        pid_t curProc = fork();
        if (-1 == curProc)
            return -1;
        else if (curProc > 0) {
            int status = 0;
            waitpid(curProc, &status, 0);

            if (WIFEXITED(status))
                return WEXITSTATUS(status);
            else
                return -1;
        }
    }
    printf("%lu\n", N);

    return 0;
}


int main(const int argc, const char* argv[]) {
    if (argc != 2)
        return -1;

    uint64_t N = strtoul(argv[1], NULL, 10);

    return StartTask(N);
}
