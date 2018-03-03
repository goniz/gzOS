#include <syscall.h>
#include <malloc.h>
#include <sys/signal.h>
#include "thread.h"
#include "kill.h"

struct libc_thread_cb {
    int (*func)(void* arg);
    void* argument;
};

__attribute__((noreturn))
static void libc_thread_kill(int tid) {
    kill(tid, SIGKILL);
    while (1);
}

__attribute__((noreturn))
static int libc_thread_main(struct libc_thread_cb* cb) {
    int tid = syscall(SYS_NR_GET_TID);

    if (!cb || !cb->func) {
        libc_thread_kill(tid);
    }

    cb->func(cb->argument);

    libc_thread_kill(tid);
}

pid_t thread_create(const char* name, int (*thread_func)(void* argument), void* argument)
{
    struct libc_thread_cb* cb = malloc(sizeof(*cb));
    if (!cb) {
        return -1;
    }

    cb->func = thread_func;
    cb->argument = argument;

    return syscall(SYS_NR_CREATE_THREAD, name, libc_thread_main, cb);
}
