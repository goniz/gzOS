#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <libc/mount.h>
#include <libc/readdir.h>

static void ls(const char* path)
{
    struct DirEntry dirent;
    int ret;
    int readdirfd = readdir_create(path);
    if (-1 == readdirfd) {
        printf("failed to create readdir handle\n");
        return;
    }

    printf("ls %s\n", path);
    while (0 < (ret = readdir_read(readdirfd, &dirent))) {
        printf("* %s%s\n", dirent.name, dirent.type == DIRENT_DIR ? "/" : "");
    }

    printf("ret: %d\n", ret);
    readdir_close(readdirfd);
}

int main(int argc, char** argv)
{
    puts("Hello PFlash!");

    int fd = open("/pflash/userdata", O_RDONLY);
    printf("fd = %d\n", fd);

    uint32_t buf[2];
    int ret = read(fd, buf, sizeof(buf));
    printf("read %d bytes: %08x%08x\n", ret, (unsigned int) buf[0], (unsigned int) buf[1]);

    close(fd);

    ret = mount("ext2", "/pflash/userdata", "/mnt/userdata");
    printf("mount ret: %d\n", ret);

    ls("/");
    ls("/pflash");
    ls("/mnt/userdata");
    ls("/mnt/userdata/test");

    return 0;
}
