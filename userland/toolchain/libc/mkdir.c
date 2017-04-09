#include <sched.h>
#include <syscall.h>

int	mkdir(const char* path, mode_t mode)
{
    return syscall(SYS_NR_MKDIR, path, mode);
}