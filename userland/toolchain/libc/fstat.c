#include <sys/stat.h>
#include <errno.h>

int fstat(int fd, struct stat *buf)
{
    errno = -ENOSYS;
    return -1;
}