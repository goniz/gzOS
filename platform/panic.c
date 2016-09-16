#include <platform/kprintf.h>
#include <unistd.h>
#include <lib/kernel/scheduler.h>
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

    kprintf("[panic]: current pid %d\n", scheduler_current_pid());
    while (1);
}
