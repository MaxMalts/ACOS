#include <stdlib.h>
#include <assert.h>
//#include <stdio.h>


char* StrChr(char* ptr, const char ch) {
    assert(ptr != NULL);

    while (*ptr != ch && *ptr != 0)
        ++ptr;

    return ptr;
}


char* StrChrRev(char* ptr, const char ch, char* strBeg) {
    assert(ptr != NULL);
    assert(strBeg != NULL);
    assert(ptr >= strBeg);

    while (*ptr != ch && ptr != strBeg) {
        --ptr;
    }

    return ptr;
}


void StrNCpy(char* dest, char* src, const int len) {
    assert(dest != NULL);
    assert(src != NULL);
    assert(len >= 0);

    for (int i = 0; i < len; ++i) {
        dest[i] = src[i];
    }
}


void CopyCurDir(char** resCursor, char* curDirBeg, char* curDirEnd) {
    assert(resCursor != NULL);
    assert(curDirBeg != NULL);
    assert(curDirEnd != NULL);

    int curDirLen = curDirEnd - curDirBeg + 1;
    if (curDirLen != 1) {    // not double slash
        StrNCpy(*resCursor, curDirBeg, curDirLen);
        *resCursor += curDirLen;
    }
}


extern void normalize_path(char* path) {
    assert(path != NULL);

    int firstIsSlash = 0;
    if ('/' == *path) {
        firstIsSlash = 1;
        ++path;
    }

    char* resCursor = path;
    char* curDirBeg = path;
    char* curDirEnd = path;

    curDirEnd = StrChr(curDirBeg, '/');
    while (*curDirEnd != 0) {
        if (curDirEnd - 1 >= path && '.' == *(curDirEnd - 1)) {
            if (curDirEnd - 2 >= path && '.' == *(curDirEnd - 2)) {
                if (resCursor - 2 < path)
                    return;    // bad input

                resCursor = StrChrRev(resCursor - 2, '/', path);
                if (resCursor != path)
                    ++resCursor;
            }

        } else {
            CopyCurDir(&resCursor, curDirBeg, curDirEnd);
        }

        curDirBeg = curDirEnd + 1;
        curDirEnd = StrChr(curDirBeg, '/');
    }

    CopyCurDir(&resCursor, curDirBeg, curDirEnd);
    *resCursor = 0;

    if (firstIsSlash) {
        --path;
    }
}


// int main() {
//     char buf[1000] = "";
//     scanf("%s", buf);
//     normalize_path(buf);
//     printf("%s", buf);
// }
