#include <unistd.h>
#include <syscall.h>

__attribute__((noreturn))
void exit(int status) {
    syscall(SYS_NR_EXIT, status);

    while (1);
}

__attribute__((noreturn))
void _exit(int status) {
    exit(status);
}