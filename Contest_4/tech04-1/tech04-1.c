#define _DEFAULT_SOURCE
#define _FILE_OFFSET_BITS 64

#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <sys/stat.h>
#include <sys/mman.h>



enum StdFile {
	STDIN = 0,
	STDOUT = 1,
	STDERR = 2
};


enum {
	MAXINTSTRLEN = 15
};


off_t FileSize(int file) {
    assert(file >= 0);

    struct stat fStat = {};
    if (fstat(file, &fStat) == -1)
        return -1;

    return fStat.st_size;
}


#pragma pack(1)
struct Item {
	int value;
	uint32_t next_pointer;
};
#pragma pack()


int StartTask(int fin, int fout) {
    assert(fin >= 0);
    assert(fout >= 0);

    off_t fSize = FileSize(fin);
    if (-1 == fSize)
        return -1;
    if (0 == fSize)
        return 0;

    char* fBuf = (char*)mmap(NULL, fSize, PROT_READ, MAP_SHARED, fin, 0);
    if (MAP_FAILED == fBuf || NULL == fBuf)
        return -1;

	struct Item* curItem = NULL;
    do {

        if (NULL == curItem)
            curItem = (struct Item*)fBuf;
        else
            curItem = (struct Item*)(fBuf + curItem->next_pointer);

		char curValStr[MAXINTSTRLEN + 1] = "";
		snprintf(curValStr, sizeof(curValStr), "%d ", curItem->value);
		ssize_t bytesWritten = write(fout, curValStr, strlen(curValStr));
		if (bytesWritten <= 0) {
            munmap(fBuf, fSize);
			return -1;
        }

        if (curItem->next_pointer >= fSize)
            return -1;

	} while (curItem->next_pointer != 0);

    munmap((void*)fBuf, fSize);
	return 0;
}


int main(const int argc, const char* argv[]) {
	if (argc != 2)
		return -1;

	int fin = open(argv[1], O_RDONLY);
	if (-1 == fin)
		return -1;

	int err = StartTask(fin, STDOUT);

	close(fin);
	return err;
}
