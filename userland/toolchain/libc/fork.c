#include <sched.h>
#include <syscall.h>

pid_t fork(void) {
    return syscall(SYS_NR_FORK);
}