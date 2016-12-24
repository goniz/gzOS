#include <syscall.h>

unsigned int sleep(unsigned int __seconds)
{
    return (unsigned int) syscall(SYS_NR_SLEEP, __seconds * 1000);
}