#include <stdio.h>
#include <syscall.h>

__attribute__((noreturn))
void _exit(int status) {
    void* ra_addr = __builtin_return_address(0);
    int result = syscall(SYS_NR_EXIT, status, ra_addr);

    if (0 != result) {
        printf("exit: failed with %d\n", result);
    }

    while (1);
}
