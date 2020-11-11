#define _DEFAULT_SOURCE
#define _FILE_OFFSET_BITS 64

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <assert.h>


off_t FileSize(int file) {
    assert(file >= 0);

    struct stat fStat = {};
    if (fstat(file, &fStat) == -1)
        return -1;

    return fStat.st_size;
}


void StartTask(const char* str, const char* substr) {
    assert(str != NULL);
    assert(substr != NULL);

    const char* foundStr = strstr(str, substr);
    while (foundStr != NULL) {
        printf("%ld\n", foundStr - str);
        foundStr = strstr(foundStr + 1, substr);
    }
}


int main(const int argc, const char* argv[]) {
    if (argc != 3)
        return -1;

    int file = open(argv[1], O_RDONLY);
    if (-1 == file)
        return -1;

    off_t fSize = FileSize(file);
    if (-1 == fSize) {
        return -1;
    }
    if (0 == fSize) {
        close(file);
        return 0;
    }


    char* fBuf = (char*)mmap(NULL, fSize, PROT_READ, MAP_SHARED, file, 0);
    if (MAP_FAILED == fBuf || NULL == fBuf)
        return -1;

    StartTask(fBuf, argv[2]);

    munmap((void*)fBuf, fSize);
    close(file);

    return 0;
}
