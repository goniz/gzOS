#ifndef USERLAND_WAITPID_H
#define USERLAND_WAITPID_H

#include <sched.h>

#ifdef __cplusplus
extern "C" {
#endif

int waitpid(pid_t pid);

#ifdef __cplusplus
}
#endif //extern "C"
#endif //USERLAND_WAITPID_H
