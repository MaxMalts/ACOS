#define _DEFAULT_SOURCE
#define _FILE_OFFSET_BITS 64

#include <sys/types.h>
#include <unistd.h>

#include <sys/stat.h>
#include <fcntl.h>

#include <unistd.h>

#include <stdio.h>

#include <string.h>

#define FALSE 0
#define TRUE 1

typedef int bool;

/*
stat(pathname, &sb);
           if ((sb.st_mode & S_IFMT) == S_IFREG) {
               // Handle regular file 
           }

stat(pathname, &sb);
           if (S_ISREG(sb.st_mode)) {
               // Handle regular file 
           }
*/

enum { 
    maxIntStrLen = 30
};

int
main(int argc, char *argv[])
{
    ssize_t in_read;
    int exit_code = 0;
    int in;
    struct stat sb;
    char pathname[100];

    int size = 0;
    char symbol;
    int ind = -1;
    bool is_read = FALSE;

    while (TRUE) {
        ind = -1;
        pathname[0] = '\0';
        is_read = FALSE;
        while ((in_read=read(0, &symbol, sizeof(symbol)))>0 ) {
            if (!is_read && ( symbol == '\n' || symbol == '\t')) {
                continue;
            } else if (is_read && ( symbol == '\n' || symbol == '\t')) {
                break;
            }
        
            is_read = TRUE;
            pathname[++ind] = symbol;
        }
		pathname[++ind] = 0;

        if (-1==in_read) {
            exit_code = 1;
            return exit_code;
        }

        printf("%s\n", pathname);

        lstat(pathname, &sb);
        if (!S_ISREG(sb.st_mode)) {
            if (in_read == 0) {
            	break;
        	}

			continue;
        }
        in = open(pathname, O_RDONLY);
        if (-1==in) {
            exit_code = 2;
            return exit_code;
        }
        size += lseek(in, 0, SEEK_END);
        close(in);

		if (in_read == 0) {
            break;
        }
    }

    char curValStr[maxIntStrLen + 1] = "";
    snprintf(curValStr, sizeof(curValStr), "%d", size);       
    write(1, curValStr, strlen(curValStr));

    return exit_code;
}