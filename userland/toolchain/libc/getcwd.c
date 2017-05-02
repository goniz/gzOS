#include <stddef.h>
#include <syscall.h>

char* getcwd(char* buf, size_t size) {
    return syscall(SYS_NR_GET_CWD, buf, size);
}