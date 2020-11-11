#define _DEFAULT_SOURCE

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

    if (S_ISREG(fStat.st_mode))
        return TRUE;

    return FALSE;
}


enum BoolError FileIsLink(const char* fName) {
    assert(fName != NULL);

    struct stat fStat = {};
    if (lstat(fName, &fStat) == -1)
        return ERROR;

    if (S_ISLNK(fStat.st_mode))
        return TRUE;

    return FALSE;
}


int main() {
    char* fName = NULL;
    size_t len = 0;

    ssize_t bytesRead = getline(&fName, &len, stdin);
    while (bytesRead >= 0) {
        if (0 == bytesRead) {
            bytesRead = getline(&fName, &len, stdin);
            continue;
        }
        if ('\n' == fName[bytesRead - 1])
            fName[bytesRead - 1] = '\0';

        enum BoolError isLink = FileIsLink(fName);
        if (ERROR == isLink)
            return -1;

        if (TRUE == isLink) {
            char* fullPath = realpath(fName, NULL);
            if (NULL == fullPath) {
                free(fullPath);
                return -1;
            }

            printf("%s\n", fullPath);
            free(fullPath);

        } else {
            enum BoolError isReg = FileIsRegular(fName);
            if (ERROR == isReg)
                return -1;

            if (TRUE == isReg) {
                const char* prefix = "link_to_";

                char* noDirName = strrchr(fName, '/');
                if (NULL == noDirName)
                    noDirName = fName;
                else
                    ++noDirName;

                char linkName[PATH_MAX] = "";
                snprintf(linkName, sizeof(linkName), "%s%s", prefix, noDirName);

                if (symlink(fName, linkName) == -1)
                    return -1;
            }
        }

        bytesRead = getline(&fName, &len, stdin);
    }

    free(fName);
    return 0;
}
