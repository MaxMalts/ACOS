#include <assert.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

int main()
{
    const char pythPrefix[] = "#!/usr/bin/python3\n"
                              "print(";
    const char pythPostfix[] = ")\n";

    char pythProg[1024 * 1024 * 5] = "";
    strncpy(pythProg, pythPrefix, sizeof(pythPrefix) - 1);
    char* cursor = pythProg + sizeof(pythPrefix) - 1;

    char curCh = 0;
    while ((curCh = getchar()) != EOF) {
        if (cursor - pythProg + sizeof(pythPostfix) > sizeof(pythProg))
            return -1;

        *cursor++ = curCh;
    }

    strncpy(cursor, pythPostfix, sizeof(pythProg) - (cursor - pythProg));

    execlp("python3", "python3", "-c", pythProg, NULL);
    return -1;
}