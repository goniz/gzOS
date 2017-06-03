#ifndef _SYS_SELECT_H
#define _SYS_SELECT_H
#ifdef __cplusplus
extern "C" {
#endif

#include <sys/time.h>
#include <sys/types.h>

int select(int nfds, fd_set* readfds, fd_set* writefds, fd_set* exceptfds, struct timeval* timeout);

#ifdef __cplusplus
}
#endif
#endif
