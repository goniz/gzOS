#include <syscall.h>

/*
 read
 Read a character to a file. `libc' subroutines will use this system routine for input from all files, including stdin
 Returns -1 on error or blocks until the number of characters have been read.
 */
int read(int file, char *ptr, int len)
{
    return (int) syscall(SYS_NR_READ, file, ptr, len);
}