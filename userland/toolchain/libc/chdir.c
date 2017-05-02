
#include <syscall.h>

int chdir(const char* path)
{
    return syscall(SYS_NR_CHDIR, path);
}