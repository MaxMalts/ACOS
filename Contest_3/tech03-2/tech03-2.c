#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
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


enum BoolError IsExecutable(int file) {
    assert(file >= 0);

    struct stat fStat = {};
    if (fstat(file, &fStat) == -1)
        return ERROR;

    if (fStat.st_mode & S_IXUSR)
        return TRUE;

    return FALSE;
}


enum BoolError CorrectHeader(int file) {
    assert(file >= 0);

    char buf[1024] = "";

    const char elfStr[] = "\x7f""ELF";
    const char interStr[] = "#!";

    ssize_t bytesRead = read(file, buf, sizeof(buf));
    if (-1 == bytesRead)
        return ERROR;

    if (bytesRead >= sizeof(elfStr) - 1 && strncmp(elfStr, buf, sizeof(elfStr) - 1) == 0) {
        return TRUE;

    } else if (bytesRead > sizeof(interStr) - 1 && strncmp(interStr, buf, sizeof(interStr) - 1) == 0) {
        char* endStr = strchr(buf, '\n');
        if (endStr != NULL) {
            struct stat fStat = {};

            *endStr = 0;
            char* interpreter = buf + 2;
            if (stat(interpreter, &fStat) == 0 && (fStat.st_mode & S_IXUSR)) {

                return TRUE;
            }

        }
    }

    return FALSE;
}


int main() {
    char fName[PATH_MAX] = "";

    while(ReadLine(STDIN, fName) > 0) {
        int curFile = open(fName, O_RDONLY);
        if (-1 == curFile)
            return -1;

        enum BoolError isExec = IsExecutable(curFile);
        enum BoolError corrHead = CorrectHeader(curFile);
        if(ERROR == isExec || ERROR == corrHead) {
            return -1;

        } else if (TRUE == isExec && FALSE == corrHead) {
            write(STDOUT, fName, strlen(fName));
            write(STDOUT, " ", 1);
        }
    }

    return 0;
}
