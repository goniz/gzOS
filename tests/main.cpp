//
// Created by gz on 3/11/16.
//

#include <stdio.h>
#include <lib/scheduler/scheduler.h>
#include <platform/malta/interrupts.h>
#include <platform/panic.h>

int test_main(int argc, const char** argv)
{
    while (1);
}

int main(int argc, const char** argv)
{
    printf("Hello!\n");

    ProcessScheduler scheduler(10, 10);
    scheduler.setDebugMode();

    printf("pid: %d\n", scheduler.createPreemptiveProcess("Test1", test_main, {}, 1024, 10));

    printf("pid: %d\n", scheduler.createPreemptiveProcess("Test2", test_main, {}, 1024, 10));

    struct user_regs regs;
    struct user_regs* new_regs = &regs;

    for (int i = 0; i < 30; i++) {
        new_regs = scheduler.onTickTimer(&scheduler, new_regs);
        print_user_regs(new_regs);
    }

    while (1);
}
