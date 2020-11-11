// mmap, т.к. файл может быть размера больше 64М,
// а нам надо передать содержимое всего файла сразу.
// Мы, грубо говоря, не можем сделать несколько вызовов exec,
// передавая каждый раз разные части входного файла.

#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

off_t FileSize(int file)
{
    assert(file >= 0);

    struct stat fStat = {};
    if (fstat(file, &fStat) == -1)
        return -1;

    return fStat.st_size;
}

off_t FileSizeStr(const char* fName)
{
    struct stat fStat = {};
    if (stat(fName, &fStat) == -1)
        return -1;

    return fStat.st_size;
}

int main(const int argc, const char* argv[])
{
    const char* fOutName = "output.txt";

    if (argc != 3)
        return -1;

    int fIn = open(argv[2], O_RDONLY);
    if (-1 == fIn)
        return -1;

    off_t fSize = FileSize(fIn);
    char* fileBuf = mmap(NULL, fSize, PROT_READ, MAP_SHARED, fIn, 0);

    pid_t process = fork();
    if (-1 == process) {
        return -1;
    } else if (0 == process) {
        int fOut = open(fOutName, O_WRONLY | O_CREAT, 0666);
        if (-1 == fOut)
            return -1;

        if (dup2(fOut, 1) == -1)
            return -1;

        if (0 == fSize)
            execlp(argv[1], argv[1], NULL);
        execlp(argv[1], argv[1], fileBuf, NULL);
    }

    int procExisStatus = 0;
    wait(&procExisStatus);

    off_t ans = FileSizeStr(fOutName);
    printf("%ld\n", ans);

    munmap(fileBuf, fSize);
    close(fIn);

    unlink(fOutName);

    return 0;
}