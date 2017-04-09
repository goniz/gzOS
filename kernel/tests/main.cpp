//
// Created by gz on 3/11/16.
//

#include <stdio.h>
#include <lib/kernel/sched/scheduler.h>
#include <platform/malta/interrupts.h>
#include <platform/panic.h>
#include <lib/kernel/initrd/load_initrd.h>
#include <lib/network/interface.h>

int test_main(void* argument)
{
    while (1);
}

int spawn_and_wait(const char* binary) {
    std::vector<const char*> shellArgs{binary};
    pid_t shellpid = syscall(SYS_NR_EXEC, binary, shellArgs.size(), shellArgs.data());
    if (-1 != shellpid) {
        syscall(SYS_NR_WAIT_PID, shellpid);
    }

    return shellpid;
}

extern "C"
int kernel_main(void* argument)
{
    printf("Hello!\n");

    interface_add("eth0", 0x01010101, 0xffffff00);

    initrd_initialize();

//    spawn_and_wait("/bin/ls");
//    spawn_and_wait("/bin/ls");
//    spawn_and_wait("/bin/ls");
    spawn_and_wait("/bin/exec_test");

    while (1);
}
