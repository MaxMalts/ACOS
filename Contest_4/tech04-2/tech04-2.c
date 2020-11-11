#define _DEFAULT_SOURCE
#define _FILE_OFFSET_BITS 64

#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>
//#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <sys/stat.h>
#include <sys/mman.h>

// Somehow uint64_t is defined as unsigned long int instead of
// unsigned long long int. I don't know why.
typedef unsigned long long int uint64_t;


enum {
	MAXULLSTRLEN = 30
};


off_t FileSize(int file) {
    assert(file >= 0);

    struct stat fStat = {};
    if (fstat(file, &fStat) == -1)
        return -1;

    return fStat.st_size;
}


enum Direction {
    up, right, down, left
};

void StartTask(char* buf, const uint64_t tableSize, const uint64_t cellWidth) {
    assert(buf != NULL);

    char formatStr[MAXULLSTRLEN + 1] = "";
    snprintf(formatStr, MAXULLSTRLEN, "%%%llullu", cellWidth);

    uint64_t curRow = 1, curColumn = 1;
    enum Direction curDir = right;
    uint64_t topFilled = 0, rightFilled = 0, bottomFilled = 0, leftFilled = 0;
    for (uint64_t i = 1; i <= tableSize * tableSize; ++i) {
        char* curPos = buf + (curRow - 1) * (cellWidth * tableSize + 1) + (curColumn - 1) * cellWidth;
        char nextChBackup = *(curPos + cellWidth);

        snprintf(curPos, cellWidth + 1, formatStr, i);
        *(curPos + cellWidth) = nextChBackup;

        switch (curDir) {
        case right:
            if (curColumn + 1 > tableSize - rightFilled) {
                curDir = down;
                ++curRow;
                ++topFilled;
            } else
                ++curColumn;
            break;

        case down:
            if (curRow + 1 > tableSize - bottomFilled) {
                curDir = left;
                --curColumn;
                ++rightFilled;
            } else
                ++curRow;
            break;

        case left:
            if (curColumn - 1 < 1 + leftFilled) {
                curDir = up;
                --curRow;
                ++bottomFilled;
            } else
                --curColumn;
            break;

        case up:
            if (curRow - 1 < 1 + topFilled) {
                curDir = right;
                ++curColumn;
                ++leftFilled;
            } else
                --curRow;
            break;

        default:
            assert(0);
        }
    }

    for (uint64_t i = 1; i <= tableSize; ++i) {
        *(buf + (i - 1) * (cellWidth * tableSize + 1) + tableSize * cellWidth) = '\n';
    }
}


int main(const int argc, const char* argv[]) {
    if (argc != 4)
        return -1;

    int file = open(argv[1], O_RDWR | O_CREAT, 0666);
    if (-1 == file)
        return -1;

    uint64_t N = strtoull(argv[2], NULL, 10);
    uint64_t W = strtoull(argv[3], NULL, 10);
    uint64_t fSize = W * N * N + N;    // last N for '\n'
    if (ftruncate(file, fSize) == -1) {
        close(file);
        return -1;
    }

    char* fBuf = (char*)mmap(NULL, fSize, PROT_READ | PROT_WRITE, MAP_SHARED, file, 0);
    if (MAP_FAILED == fBuf || NULL == fBuf)
        return -1;

    assert((W > 0 && N > 0) || 0 == N);

    StartTask(fBuf, N, W);

    munmap((void*)fBuf, fSize);
    close(file);

    return 0;
}
