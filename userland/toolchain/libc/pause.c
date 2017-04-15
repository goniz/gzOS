#include <unistd.h>
#include <syscall.h>
#include <sys/signal.h>

int pause(void)
{
    return syscall(SYS_NR_SIGNAL, getpid(), SIGSTOP);
}