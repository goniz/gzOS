#include "uart_cbus.h"
#include <platform/interrupts.h>
#include <platform/cpu.h>
#include "clock.h"
#include "malta.h"
#include "pm.h"
#include "tlb.h"

extern void system_init(int argc, char **argv, char **envp);

void platform_init(int argc, char **argv, char **envp)
{
	uart_init();
	uart_puts("in platform_init!!\n");

	platform_read_cpu_config();
    platform_dump_additional_cpu_info();

    pm_init();
    tlb_init();
	interrupts_init();
	clock_init();

	system_init(argc, argv, envp);
}
