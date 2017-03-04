#include <unistd.h>
#include <syscall.h>

int dup2(int old_fd, int new_fd)
{
    return syscall(SYS_NR_DUP, old_fd, new_fd);
}