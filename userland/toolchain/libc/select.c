#include <syscall.h>
#include "select.h"

int select(int nfds, fd_set* readfds, fd_set* writefds, fd_set* exceptfds, struct timeval* timeout)
{
    return syscall(SYS_NR_SELECT, nfds, readfds, writefds, exceptfds, timeout);
}