#include <syscall.h>
#include "waitpid.h"

int waitpid(pid_t pid) {
    return syscall(SYS_NR_WAIT_PID, pid);
}
