#include <stdio.h>
#include <stdarg.h>

long int syscall(long int number, ...)
{
    // NOTE: apparently the vaargs things needs a real stack frame to work (??)
    __attribute__((unused))
    volatile char tmp[8] = {0};
    va_list arg;

    va_start(arg, number);
    asm volatile(
        "\tmove $a0, %0\n"
        "\tmove $a1, %1\n"
        "\tnop\n"
        "\tsyscall\n"
        "\tnop\n"
        : : "r"(number), "r"(arg)
    );
    va_end(arg);

    register int v0 asm("v0");
    return v0;
}