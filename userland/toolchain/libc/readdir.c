#include <unistd.h>
#include <syscall.h>
#include <stdio.h>
#include "readdir.h"

int readdir_create(const char *path) {
    return syscall(SYS_NR_READDIR, path);
}

int readdir_read(int fd, struct DirEntry *dirent) {
    int ret = read(fd, dirent, sizeof(*dirent));
    if (0 == ret) {
        return 0;
    }

    if (sizeof(*dirent) != ret) {
        return -1;
    }

    return ret;
}

int readdir_close(int fd) {
    return close(fd);
}
