#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <libc/traceme.h>


int main(int argc, char **argv) {
    puts("Hello World!");

    traceme(1);

    int fd = open("/tmp/file1", O_CREAT | O_WRONLY);
    if (-1 == fd) {
        return 1;
    }

    write(fd, "3\n", 2);

    dup2(fd, 10);
    dup2(10, 11);
    close(fd);

    write(10, "10\n", 3);
    close(10);

    write(11, "11\n", 3);
    close(11);

    return 0;
}
