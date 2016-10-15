#include <sys/types.h>
#include <syscall.h>

int open(const char *pathname, int flags, mode_t mode)
{
    return syscall(SYS_NR_OPEN, pathname, flags, mode);
}