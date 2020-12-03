#define _XOPEN_SOURCE 700

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

enum { STDIN = 0, STDOUT = 1, STDERR = 2 };

void OutputPid()
{
    pid_t curPid = getpid();
    printf("%d\n", curPid);
    fflush(stdout);
}

sig_atomic_t num = 0;

void UsrHandler(int signal)
{
    sigset_t sigset;
    sigfillset(&sigset);
    sigprocmask(SIG_BLOCK, &sigset, NULL);

    if (SIGUSR1 == signal) {
        ++num;
    } else {
        num *= -1;
    }

    char buf[100] = "";
    snprintf(buf, sizeof(buf), "%d\n", num);

    write(STDOUT, buf, strlen(buf));

    sigprocmask(SIG_UNBLOCK, &sigset, NULL);
}

void TermHandler(int signal)
{
    _exit(0);
}

int main()
{
    struct sigaction action = {};
    action.sa_handler = UsrHandler;
    action.sa_flags = SA_RESTART;
    if (sigaction(SIGUSR1, &action, NULL) == -1) {
        return -1;
    }
    if (sigaction(SIGUSR2, &action, NULL) == -1) {
        return -1;
    }

    action.sa_handler = TermHandler;
    if (sigaction(SIGTERM, &action, NULL) == -1) {
        return -1;
    }

    scanf("%d", &num);
    OutputPid();

    while (1) {}

    return -1;
}