#include <stddef.h>
#include <syscall.h>
#include "socket.h"

int bind(int __fd, const sockaddr_t*__addr, size_t __len)
{
    (void)__len;
    return syscall(SYS_NR_BIND, __fd, __addr);
}