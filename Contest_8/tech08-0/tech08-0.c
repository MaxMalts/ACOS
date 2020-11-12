#define _XOPEN_SOURCE 700

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <signal.h>

void OutputPid() {
	pid_t curPid = getpid();
	printf("%d\n", curPid);
	fflush(stdout);
}

sig_atomic_t nInts = 0;

void IntHandler(int signal) {
	++nInts;
}

void TermHandler(int signal) {
	printf("%d\n", nInts);
	fflush(stdout);
	exit(0);
}

int main() {
	struct sigaction action = {};
	action.sa_flags = 0;
	action.sa_handler = IntHandler;
	if (sigaction(SIGINT, &action, NULL) == -1) {
		return -1;
	}

	action.sa_handler = TermHandler;
	if (sigaction(SIGTERM, &action, NULL) == -1) {
		return -1;
	}

	OutputPid();

	while (1) {}

	return -1;
}