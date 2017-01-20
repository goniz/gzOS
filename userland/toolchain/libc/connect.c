#include <syscall.h>
#include "socket.h"

int connect(int fd, const sockaddr_t* addr)
{
    return syscall(SYS_NR_CONNECT, fd, addr);
}