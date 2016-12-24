#include <syscall.h>
#include "socket.h"

int recv(int __fd, void *__buf, size_t __n, int __flags)
{
    (void)__flags;
    return syscall(SYS_NR_READ, __fd, __buf, __n);
}