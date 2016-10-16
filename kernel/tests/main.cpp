//
// Created by gz on 3/11/16.
//

#include <stdio.h>
#include <lib/kernel/sched/scheduler.h>
#include <platform/malta/interrupts.h>
#include <platform/panic.h>

int test_main(void* argument)
{
    while (1);
}

extern "C"
int kernel_main(void* argument)
{
    printf("Hello!\n");

    Scheduler scheduler;
    scheduler.setDebugMode();

    printf("thread: %p\n", scheduler.createKernelThread("Test1", test_main, nullptr, 1024));

    printf("thread: %p\n", scheduler.createKernelThread("Test2", test_main, nullptr, 1024));

    struct user_regs regs;
    struct user_regs* new_regs = &regs;

    for (int i = 0; i < 30; i++) {
        new_regs = scheduler.onTickTimer(&scheduler, new_regs);
        print_user_regs(new_regs);
    }

    while (1);
}
