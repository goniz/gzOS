#include <stddef.h>
#include <syscall.h>
#include "socket.h"

int accept(int __fd, sockaddr_t* __addr, size_t* __len)
{
    return syscall(SYS_NR_ACCEPT, __fd, __addr, __len);
}