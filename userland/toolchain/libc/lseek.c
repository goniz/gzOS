#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <syscall.h>

off_t lseek(int fd, off_t offset, int whence)
{
    return syscall(SYS_NR_LSEEK, fd, offset, whence);
}

