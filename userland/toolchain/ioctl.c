#include <syscall.h>
#include <stdarg.h>
#include "ioctl.h"

int ioctl(int fildes, int request, ... /* args */) {
    va_list args;
    va_start(args, request);
    int result = syscall(SYS_NR_IOCTL, fildes, request, args);
    va_end(args);

    return result;
}
