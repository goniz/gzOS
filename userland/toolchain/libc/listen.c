#include <syscall.h>

int listen(int __fd, int __backlog)
{
    return syscall(SYS_NR_LISTEN, __fd, __backlog);
}