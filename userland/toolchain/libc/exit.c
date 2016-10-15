#include <unistd.h>
#include <syscall.h>

__attribute__((noreturn))
void _exit(int status) {
    syscall(SYS_NR_EXIT);

    while (1);
}