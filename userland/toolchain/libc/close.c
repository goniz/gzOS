#include <errno.h>
#include <syscall.h>

int close(int file)
{
    syscall(SYS_NR_CLOSE, file);
}