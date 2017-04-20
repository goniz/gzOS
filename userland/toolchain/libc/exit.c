#include <unistd.h>
#include <syscall.h>

__attribute__((noreturn))
void exit(int status) {
    void* ra_addr = __builtin_return_address(0);
    int ret = syscall(SYS_NR_EXIT, status, ra_addr);

    printf("sys_exit ret was %d\n", ret);

    while (1);
}

__attribute__((noreturn))
void _exit(int status) {
    exit(status);
}
