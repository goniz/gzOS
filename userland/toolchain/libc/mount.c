#include <syscall.h>

int mount(const char* fstype, const char* source, const char* destination)
{
    return syscall(SYS_NR_MOUNT, fstype, source, destination);
}