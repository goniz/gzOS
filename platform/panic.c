#include <platform/kprintf.h>
#include <unistd.h>
#include "interrupts.h"

__attribute__((noreturn))
void panic(const char* fmt, ...)
{
    va_list arg;

    interrupts_disable();

    va_start(arg, fmt);
    kprintf("[panic]: ");
	vkprintf(fmt, arg);
    kprintf("\n");
    va_end(arg);

    kprintf("[panic]: current pid %d\n", getpid());
    while (1);
}
