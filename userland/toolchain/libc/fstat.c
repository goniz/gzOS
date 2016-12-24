#include <sys/stat.h>
#include <errno.h>

#if defined(__mips__)
__attribute__((nomips16))
#endif
int fstat(int fd, struct stat *buf)
{
    errno = -ENOSYS;
    return -1;
}
