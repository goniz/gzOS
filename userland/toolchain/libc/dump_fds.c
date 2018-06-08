#include "dump_fds.h"
#include <syscall.h>

void dump_fds(void) {
    syscall(SYS_NR_DUMP_FDS);
}
