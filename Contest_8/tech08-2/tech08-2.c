#define _XOPEN_SOURCE 700

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

sig_atomic_t globSignal = 0;

void SigHandler(int signal)
{
    globSignal = signal;
}

int main(const int argc, const char* argv[])
{
    struct sigaction action;
	action.sa_handler = SigHandler;
	action.sa_flags = SA_RESTART;

    for (int i = SIGRTMIN; i < SIGRTMIN + argc; ++i) {
        sigaction(i, &action, NULL);
	}

	for (int i = 0; i < SIGRTMIN; ++i) {
        signal(i, SIG_IGN);
	}

    for (int i = SIGRTMIN + argc; i <= SIGRTMAX; ++i) {
        signal(i, SIG_IGN);
	}

    while (1) {
        pause();
        sig_atomic_t curNum = globSignal - SIGRTMIN;
        if (0 == curNum) {
            return 0;
		}

        FILE* curFile = fopen(argv[curNum], "r");
        if (NULL == curFile) {
            return -1;
		}

        char* str = NULL;
        size_t strSize = 0;
        ssize_t strLen = getline(&str, &strSize, curFile);
		if (-1 == strLen) {
			return -1;
		}
		if (strLen > 0 && '\n' == str[strLen - 1]) {
			str[strLen - 1] = 0;
		}
		printf("%s\n", str);

        fclose(curFile);
		free(str);

        fflush(stdout);
    }
}