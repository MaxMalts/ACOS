#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdint.h>


int main() {
    uint64_t ans = 0;   // uint64_t is defined as unsigned long, not unsigned long long and is 32 bit.

    pid_t curProc = 0;
    do {
        ++ans;

        curProc = fork();
        if (0 == curProc)
            return 0;
    } while (curProc != -1);

    for (uint64_t i = 1; i < ans; ++i) {
        int status = 0;
        wait(&status);
    }

    printf("%lu", ans);
}
