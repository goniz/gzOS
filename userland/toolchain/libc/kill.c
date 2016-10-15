
#include <syscall.h>

/*
 kill
 Send a signal. Minimal implementation:
 */
int kill(int pid, int sig)
{
    return (int) syscall(SYS_NR_SIGNAL, pid, sig);
}
