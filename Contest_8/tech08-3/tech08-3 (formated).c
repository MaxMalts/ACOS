#define _XOPEN_SOURCE 700

#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

void SigHandler(int signal, siginfo_t* info, void* context)
{
    int val = info->si_value.sival_int;
    if (0 == val) {
        exit(0);
    }

    union sigval sigVal;
    sigVal.sival_int = val - 1;
    sigqueue(info->si_pid, signal, sigVal);
}

void TermHandler(int signal)
{
    exit(0);
}

int main()
{
    struct sigaction action = {};
    action.sa_sigaction = SigHandler;
    action.sa_flags = SA_SIGINFO;

    for (int i = 0; i < SIGRTMAX; ++i) {
        if (SIGRTMIN == i) {
            sigaction(i, &action, NULL);
        } else {
            signal(i, SIG_IGN);
        }
    }

    signal(SIGTERM, TermHandler);

    while (1) {
        pause();
    }
}