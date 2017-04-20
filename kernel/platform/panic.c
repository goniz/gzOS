#include <platform/kprintf.h>
#include <unistd.h>
#include <lib/kernel/proc/Scheduler.h>
#include <lib/kernel/proc/proc.h>
#include "interrupts.h"

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
    if (scheduler_get()) {
        pid = scheduler_current_pid();
    }

    kprintf("[panic]: current pid %d\n", pid);
    while (1);
}
