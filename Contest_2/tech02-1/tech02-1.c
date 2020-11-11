#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
//#include <stdio.h>


enum StdFile {
	stdinput = 0,
	stdOutput = 1, 
	stdError = 2
};


enum {
	maxIntStrLen = 15
};


#pragma pack(1)
struct Item {
	int value;
	uint32_t next_pointer;
};
#pragma pack()


int StartTask(int fin, int fout) {

	struct Item curItem = {0, 0};
	do {

		if(lseek(fin, curItem.next_pointer, SEEK_SET) == -1)
			return -1;

		ssize_t bytesRead = read(fin, &curItem, sizeof(curItem));
		if (bytesRead != sizeof(curItem))
			return -1;

		char curValStr[maxIntStrLen + 1] = "";
		snprintf(curValStr, sizeof(curValStr), "%d ", curItem.value);
		ssize_t bytesWritten = write(fout, curValStr, strlen(curValStr));
		if (bytesWritten <= 0)
			return -1;

	} while(curItem.next_pointer != 0);

	return 0;
}


int main(const int argc, const char* argv[]) {
	if (argc != 2)
		return -1;

	int fin = open(argv[1], O_RDONLY);
	if(-1 == fin)
		return -1;

	int err = StartTask(fin, stdOutput);

	close(fin);
	return err;
}