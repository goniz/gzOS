#include <syscall.h>
#include "traceme.h"

int traceme(int state) {
    return syscall(SYS_NR_TRACEME, state);
}
