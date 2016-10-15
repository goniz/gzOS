#include <syscall.h>

/*
 write
 Write a character to a file. `libc' subroutines will use this system routine for output to all files, including stdout
 Returns -1 on error or number of bytes sent
 */
int write(int file, const char *buf, int len)
{
    return (int) syscall(SYS_NR_WRITE, file, buf, len);
}