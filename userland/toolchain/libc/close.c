#include <errno.h>

int close(int file)
{
    errno = -ENOENT;
    return -1;
}