#include <assert.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>


int WriteProgram(const char progFName[]) {
	assert(progFName != NULL);

	const char progPrefix[] =
		"#include <stdio.h>\n"
        "int main() {\n"
			"\tint ans = (";
    const char progPostfix[] = ");\n"
			"\tprintf(\"%d\", ans);\n"
			"\treturn 0;\n"
		"}";

	int progFile = open(progFName, O_WRONLY | O_CREAT, 0666);
	if (-1 == progFile)
		return -1;
	
    write(progFile, progPrefix, sizeof(progPrefix) - 1);

    char curCh = 0;
    while ((curCh = getchar()) != EOF)
        write(progFile, &curCh, sizeof(curCh));

    write(progFile, progPostfix, sizeof(progPostfix) - 1);
    close(progFile);

	return 0;
}


int CompileCProgram(const char progFName[], const char execFName[]) {
	assert(progFName != NULL);
	assert(execFName != NULL);

	pid_t procPid = fork();
	if (-1 == procPid)
		return -1;
	else if (0 == procPid) {
		execlp("gcc", "gcc", progFName, "-o", execFName, NULL);
		exit(-1);
	}

	int status = 0;
	waitpid(procPid, &status, 0);

	if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
		return 0;	
	return -1;
}


int RunProgram(const char execFName[]) {
	assert(execFName != NULL);

	pid_t procPid = fork();
	if (-1 == procPid)
		return -1;
	if (0 == procPid) {
		execl(execFName, execFName, NULL);
		exit(-1);
	}

	int status = 0;
	waitpid(procPid, &status, 0);

	if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
		return 0;
	return -1;
}


int main() {
    const char progFName[] = "prog.c";
	const char execFName[] = "prog.out";

	if (WriteProgram(progFName) != 0)
		return -1;

	if (CompileCProgram(progFName, execFName) != 0)
		return -1;

	if (RunProgram(execFName) != 0)
		return -1;

	unlink(progFName);
	unlink(execFName);

    return 0;
}