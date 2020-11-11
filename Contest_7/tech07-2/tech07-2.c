#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>


enum StdFds{
	STDIN = 0,
	STDOUT = 1,
	STDERR = 2
};


pid_t CompileGcc(int pipefd[2], const char* progFName) {
	assert(progFName != NULL);

	pid_t process = fork();
	if (-1 == process)
		return -1;
	else if (0 == process) {
		if (dup2(pipefd[1], STDERR) == -1)
			exit(-1);
		
		close(pipefd[0]);
		close(pipefd[1]);
		
		execlp("gcc", "gcc", progFName, "-o", "prog.out", NULL);
		exit(-1);
	}
}


void ParseGcc() {
	int prevWarningStr = 0;
	int prevErrorStr = 0;
	int warningStrs = 0;
	int errorStrs = 0;

	 while(1) {
        int curStr = 0;
        int res = scanf("%*[^:]:%d:%*d: ", &curStr);

		if (res == EOF)
            break;
		
		if (res == 0)
            continue;

        char nextCh = 0;
		if (scanf("warning%c", &nextCh) == 1) {
            if (curStr != prevWarningStr) {
            	prevWarningStr = curStr;
                ++warningStrs;
			}

		} else if (scanf("error%c", &nextCh) == 1) {
            if (curStr != prevErrorStr) {
            	prevErrorStr = curStr;
                ++errorStrs;
			}
        }
    }

	printf("%d %d\n", errorStrs, warningStrs);
}


int main(const int argc, const char* argv[]) {
	if (argc != 2)
		return -1;

	int pipefd[2] = {0};
	if (pipe(pipefd) == -1)
		return -1;
	
	pid_t process = CompileGcc(pipefd, argv[1]);
	if (-1 == process)
		return -1;

	dup2(pipefd[0], STDIN);
	close(pipefd[0]);
	close(pipefd[1]);

	ParseGcc();

	int status = 0;
	wait(&status);

	return 0;
}