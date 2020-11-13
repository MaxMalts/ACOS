#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>


enum { STDIN = 0, STDOUT = 1, STDERR = 2 };


int main(const int argc, const char* argv[]) {
	if (argc != 3) {
		return -1;
	}

	int pipefd[2] = {0};
	if (pipe(pipefd) == -1) {
		return -1;
	}

	int fd = open(argv[2], O_RDONLY);
	if (-1 == fd) {
		return -1;
	}

	pid_t process = fork();
	if (-1 == process) {
		return -1;
	} else if (0 == process) {
		dup2(fd, STDIN);
		dup2(pipefd[1], STDOUT);

		close(pipefd[0]);
		close(pipefd[1]);

		execlp(argv[1], argv[1], NULL);
	}

	close(pipefd[1]);

	int ans = 0;
	char curCh = 0;
	while (read(pipefd[0], &curCh, 1) > 0) {
		++ans;
	}

	printf("%d\n", ans);

	close(fd);
	close(pipefd[0]);
	
	return 0;
}