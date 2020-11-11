#include <sys/syscall.h>

.intel_syntax noprefix
.global _start


#define BUFFER_SIZE 1024 * 1024
#define STDIN 0
#define STDOUT 1
#define STDERR 2

.text

_start:
	read_loop:
		mov eax, SYS_read
		mov edi, STDIN
		mov esi, offset buffer
		mov edx, BUFFER_SIZE
		syscall
		mov ebx, eax    // bytes read

		mov eax, SYS_write
		mov edi, STDOUT
		mov esi, offset buffer
		mov edx, ebx
		syscall

		cmp ebx, BUFFER_SIZE
		je read_loop

	mov eax, SYS_exit
	mov edi, 0
	syscall


.data
buffer:
	.zero BUFFER_SIZE

test:
	.ascii "there\n"
