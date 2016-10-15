
#include <sched.h>
#include <syscall.h>

/*
 getpid
 Process-ID; this is sometimes used to generate strings unlikely to conflict with other processes. Minimal implementation, for a system without processes:
 */

pid_t getpid(void)
{
    return syscall(SYS_NR_GET_PID);
}
