#include "uart_cbus.h"
#include <platform/interrupts.h>
#include "clock.h"

extern void system_init(int argc, char **argv, char **envp);

void platform_init(int argc, char **argv, char **envp)
{
	uart_init();

	uart_puts("in platform_init!!\n");

	interrupts_init();
	clock_init();

	system_init(argc, argv, envp);
}
