
#include <syscall.h>

int pipe(int fildes[2])
{
    return syscall(SYS_NR_PIPE, fildes);
}