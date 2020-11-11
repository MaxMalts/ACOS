#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <stdint.h>
#include <sys/wait.h>
#include <assert.h>



int WordRead() {
    char curCh = ' ';
    while (' ' == curCh || '\n' == curCh) {
        curCh = getchar();
        if (EOF == curCh)
            return 0;
    }

    while (curCh != ' ' && curCh != '\n' && curCh != EOF) {
        curCh = getchar();
    }

    return 1;
}


int main() {
    int ans = 0;

    int wordRead = 0;
    do {
        pid_t curProc = fork();
        if (-1 == curProc)
            return -1;
        else if (0 == curProc)
            return WordRead();

        int status = 0;
        waitpid(curProc, &status, 0);
        if (WIFEXITED(status))
            wordRead = WEXITSTATUS(status);
        else
            return -1;

        if (wordRead)
            ++ans;
    } while (wordRead);

    printf("%d\n", ans);

    return 0;
}
