//
// Created by gz on 3/11/16.
//

#include <stdio.h>
#include <lib/kernel/proc/Scheduler.h>
#include <platform/malta/interrupts.h>
#include <platform/panic.h>
#include <lib/kernel/initrd/load_initrd.h>
#include <lib/network/interface.h>

extern "C"
int kernel_main(void* argument)
{
    printf("Hello!\n");

    interface_add("eth0", 0x01010101, 0xffffff00);

    initrd_initialize();

    while (1);
}
