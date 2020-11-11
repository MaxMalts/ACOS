#include <sys/syscall.h>
#include <sys/types.h>

long syscall(long number, ...);

void _start() {
	const ssize_t maxBufSize = 1024 * 1024;
	char buf[maxBufSize];
	
	ssize_t curCopied = 0;
	do {
		curCopied = syscall(SYS_read, 0, buf, maxBufSize);
		syscall(SYS_write, 1, buf, curCopied);
	} while (curCopied == maxBufSize);

	syscall(SYS_exit, 0);
}
