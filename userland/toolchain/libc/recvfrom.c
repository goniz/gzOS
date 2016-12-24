#include <syscall.h>
#include "socket.h"

int recvfrom(int __fd, void * __buf, size_t __n,
             int __flags, sockaddr_t* __addr,
             size_t * __addr_len)
{
    if (__addr_len) {
        *__addr_len = sizeof(sockaddr_t);
    }

    return syscall(SYS_NR_RECVFROM, __fd, __buf, __n, __addr);
}