#include <stddef.h>
#include <syscall.h>

int send(int __fd, const void *__buf, size_t __n, int __flags)
{
    (void)__flags;
    return syscall(SYS_NR_WRITE, __fd, __buf, __n);
}