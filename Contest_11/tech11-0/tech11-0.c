#define FUSE_USE_VERSION 30

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

off_t FileSize(int file)
{
    assert(file >= 0);

    struct stat fStat = {};
    if (fstat(file, &fStat) == -1)
        return -1;

    return fStat.st_size;
}

enum { nFilesMax = 10000 };

struct Filesystem {
    int srcSize;
    char* srcBuf;

    int nFiles;
    char* nameOffsets[nFilesMax];
    char names[nFilesMax][5000];
    int sizes[nFilesMax];
    char* dataOffsets[nFilesMax];
} filesystem;

int InitFilesystem(const char* src)
{
    assert(src != NULL);

    int srcFile = open(src, O_RDONLY);
    if (-1 == srcFile) {
        return -1;
    }

    filesystem.srcSize = FileSize(srcFile);
    if (-1 == filesystem.srcSize) {
        close(srcFile);
        return -1;
    }

    filesystem.srcBuf = (char*)mmap(
        NULL, filesystem.srcSize, PROT_READ, MAP_PRIVATE, srcFile, 0);
    if (MAP_FAILED == filesystem.srcBuf || NULL == filesystem.srcBuf) {
        close(srcFile);
        return -1;
    }

    close(srcFile);

    int scaned = sscanf(filesystem.srcBuf, "%d", &filesystem.nFiles);
    assert(scaned > 0);

    char* sizeStart = filesystem.srcBuf;
    for (int i = 0; i < filesystem.nFiles; ++i) {
        filesystem.nameOffsets[i] = strchr(sizeStart, '\n') + 1;
        sizeStart = strchr(filesystem.nameOffsets[i], ' ') + 1;
        sscanf(sizeStart, "%d", &filesystem.sizes[i]);

        strncpy(
            filesystem.names[i],
            filesystem.nameOffsets[i],
            sizeStart - 1 - filesystem.nameOffsets[i]);
    }

    filesystem.dataOffsets[0] = strchr(sizeStart, '\n') + 2;
    assert('\n' == *(filesystem.dataOffsets[0] - 1));

    for (int i = 1; i < filesystem.nFiles; ++i) {
        filesystem.dataOffsets[i] =
            filesystem.dataOffsets[i - 1] + filesystem.sizes[i];
    }

    return 0;
}

int IndByName(const char* path)
{
    for (int i = 0; i < filesystem.nFiles; ++i) {
        if (strcmp(filesystem.names[i], path) == 0) {
            return i;
        }
    }

    return -1;
}

int MyStat(const char* path, struct stat* st, struct fuse_file_info* fi)
{
    if ('/' == path[0]) {
        ++path;
    }

    if ('\0' == path[0]) {
        st->st_mode = 0444 | S_IFDIR;
        st->st_nlink = 2;
        return 0;
    }

    int ind = IndByName(path);
    if (-1 == ind) {
        return -ENOENT;
    }

    st->st_mode = 0444 | S_IFREG;
    st->st_nlink = 1;
    st->st_size = filesystem.sizes[ind];
    return 0;
}

int MyReaddir(
    const char* path,
    void* out,
    fuse_fill_dir_t filler,
    off_t off,
    struct fuse_file_info* fi,
    enum fuse_readdir_flags flags)
{
    if ('/' == path[0]) {
        ++path;
    }

    if ('\0' != path[0]) {
        return -ENOENT;
    }

    filler(out, ".", NULL, 0, 0);
    filler(out, "..", NULL, 0, 0);

    for (int i = 0; i < filesystem.nFiles; ++i) {
        filler(out, (const char*)filesystem.names[i], NULL, 0, 0);
    }

    return 0;
}

int MyRead(
    const char* path,
    char* out,
    size_t size,
    off_t off,
    struct fuse_file_info* fi)
{
    if ('/' == path[0]) {
        ++path;
    }

    if ('\0' == path[0]) {
        return -1;
    }

    int ind = IndByName(path);
    if (-1 == ind) {
        return -ENOENT;
    }

    if (off > filesystem.sizes[ind]) {
        return 0;
    }

    if (off + size > filesystem.sizes[ind]) {
        size = filesystem.sizes[ind] - off;
    }

    memcpy(out, filesystem.dataOffsets[ind] + off, size);

    return size;
}

int main(int argc, char* argv[])
{
    struct fuse_args fuseArgs = FUSE_ARGS_INIT(argc, argv);

    char* src = NULL;

    struct fuse_opt addOptions[] = {{"--src %s", 0, 0}, {NULL, 0, 0}};

    fuse_opt_parse(&fuseArgs, &src, addOptions, NULL);

    if (src) {
        InitFilesystem(src);
    }

    struct fuse_operations operations = {
        .getattr = MyStat, .readdir = MyReaddir, .read = MyRead};
    int ret = fuse_main(fuseArgs.argc, fuseArgs.argv, &operations, NULL);

    munmap(filesystem.srcBuf, filesystem.srcSize);

    return ret;
}