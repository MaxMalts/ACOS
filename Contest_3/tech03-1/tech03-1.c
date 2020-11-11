#define _DEFAULT_SOURCE
#define _FILE_OFFSET_BITS 64

#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <stdint.h>
#include <sys/stat.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <linux/limits.h>


enum StdFiles {
    STDIN = 0,
    STDOUT = 1,
    STDERR = 2
};

enum {
    MAXULLSTRLEN = 30
};

enum BoolError {
    ERROR = -1,
    FALSE = 0,
    TRUE = 1
};


ssize_t ReadLine(int file, char buf[]) {
    assert(file >= 0);
    assert(buf != NULL);

    char curCh = 0;

    int cursor = 0;
    ssize_t bytesRead = read(file, &curCh, sizeof(curCh));
    while(bytesRead > 0 && curCh != '\n' && curCh != 0) {
        buf[cursor] = curCh;
        ++cursor;
        bytesRead = read(file, &curCh, sizeof(curCh));
    }
    if (-1 == bytesRead)
        return -1;

    buf[cursor] = 0;

    return cursor;
}


enum BoolError FileIsRegular(const char* fName) {
    assert(fName != NULL);

    struct stat fStat = {};
    if (lstat(fName, &fStat) == -1)
        return ERROR;

    return S_ISREG(fStat.st_mode);
}


off_t FileSize(const char* fName) {
    assert(fName != NULL);

    struct stat fStat = {};
    if (lstat(fName, &fStat) == -1)
        return -1;

    return fStat.st_size;
}


int main() {
    char fName[PATH_MAX] = "";

    unsigned long long ans = 0;    // Somehow uint64_t is defined as unsigned long int, not unsigned long long int. I don't know why
    while (ReadLine(STDIN, fName) > 0) {
        enum BoolError isReg = FileIsRegular(fName);
        if (ERROR == isReg)
            return -1;

        if (TRUE == isReg)
            ans += FileSize(fName);
    }

    char ansStr[MAXULLSTRLEN + 1] = "";
    snprintf(ansStr, MAXULLSTRLEN + 1, "%llu", ans);
    write(STDOUT, ansStr, strlen(ansStr));

    return 0;
}
