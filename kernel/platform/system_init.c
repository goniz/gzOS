#include <stdio.h>
#include <lib/kernel/sched/scheduler.h>
#include <platform/pci/pci.h>
#include <platform/drivers.h>
#include "kprintf.h"

static void initialize_stdio(void);
static void invoke_constructors(void);

extern int __init_array_start;
extern int __init_array_end;

extern int kernel_main(void *argument);
void system_init(int argc, const char **argv, const char **envp)
{
    initialize_stdio();
    invoke_constructors();

    drivers_init();
    platform_pci_driver_probe();

    scheduler_run_main(kernel_main, NULL);
    interrupts_enable_all();

    while(1);
}

static void initialize_stdio(void)
{
    setvbuf(stdin, NULL, _IONBF, 0);
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);
}

static void invoke_constructors(void)
{
    typedef void (*func_t)(void);
    int* init_array = &__init_array_start;
    int* init_array_end = &__init_array_end;

    /*
     *  Call C++ constructors
     */
    printf("\nInvoking C++ static constructors!\n");
    while (init_array < init_array_end) {
        func_t func = (func_t)(*init_array++);
//        printf("Invoking ctor at %p\r\n", func);
        func();
    }
}

