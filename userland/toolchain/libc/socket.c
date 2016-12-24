#include <syscall.h>
#include "socket.h"

int socket(int __domain, int __type, int __protocol)
{
    return syscall(SYS_NR_SOCKET, __domain, __type, __protocol);
}