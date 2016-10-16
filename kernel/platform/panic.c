#include <platform/kprintf.h>
#include <unistd.h>
#include <lib/kernel/sched/scheduler.h>
#include "interrupts.h"

extern void* scheduler(void);

static int is_panicing = 0;

__attribute__((noreturn))
void panic(const char* fmt, ...)
{
    va_list arg;

    interrupts_disable();
    if (is_panicing) {
        while (1);
    }

    is_panicing = 1;

    va_start(arg, fmt);
    kprintf("[panic]: ");
	vkprintf(fmt, arg);
    kprintf("\n");
    va_end(arg);

    pid_t pid = -1;
    if (scheduler()) {
        pid = scheduler_current_pid();
    }

    kprintf("[panic]: current pid %d\n", pid);
    while (1);
}
