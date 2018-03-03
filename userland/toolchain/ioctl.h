#ifndef USERLAND_IOCTL_H
#define USERLAND_IOCTL_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    FD_GET_BLOCKING = 0,
    FD_SET_BLOCKING = 1
};

int ioctl(int fildes, int request, ... /* args */);

#ifdef __cplusplus
}
#endif //extern "C"
#endif //USERLAND_IOCTL_H
