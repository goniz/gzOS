#include <errno.h>
#include <syscall.h>

int close(int file)
{
    return syscall(SYS_NR_CLOSE, file);
}