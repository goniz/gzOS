
#include <stddef.h>
#include <syscall.h>
#include "socket.h"

int sendto(int __fd, const void *__buf, size_t __n,
           int __flags, const sockaddr_t* __addr,
           size_t __addr_len)
{
    return syscall(SYS_NR_SENDTO, __fd, __buf, __n, __addr);
}