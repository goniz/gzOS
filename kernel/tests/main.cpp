//
// Created by gz on 3/11/16.
//

#include <stdio.h>
#include <lib/kernel/proc/Scheduler.h>
#include <lib/kernel/initrd/load_initrd.h>
#include <lib/network/interface.h>
#include <gtest/gtest.h>
#include <lib/kernel/signals.h>

extern int __gtest_ctor_start;
extern int __gtest_ctor_end;

static void invoke_constructors(void)
{
    typedef void (*func_t)(void);
    int* init_array = &__gtest_ctor_start;
    int* init_array_end = &__gtest_ctor_end;

    /*
     *  Call C++ constructors
     */
    printf("\nInvoking gtest static constructors!\n");
    while (init_array < init_array_end) {
        func_t func = (func_t)(*init_array++);
        printf("Invoking ctor at %p\r\n", func);
        func();
    }
}

extern "C"
int kernel_main(void* argument)
{
    initrd_initialize();
    interface_add("eth0", 0x01010101, 0xffffff00);

    int argc = 1;
    char* argv[] = {
        (char*)"tests",
        NULL
    };

    invoke_constructors();

    testing::InitGoogleTest(&argc, argv);

    int retval = RUN_ALL_TESTS();
    printf("retval: %d\n", retval);

    syscall(SYS_NR_SIGNAL, getpid(), SIG_STOP);
    return 0;
}
