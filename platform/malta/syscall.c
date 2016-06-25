#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>

int syscall(int number, ...)
{
    va_list arg;
    va_start(arg, number);
    __attribute__((unused))
    register uint32_t a0 asm("a0") = (uint32_t)number;

    __attribute__((unused))
    register uint32_t a1 asm("a1") = (uint32_t)arg;

	asm volatile("syscall");
    va_end(arg);

    __attribute__((unused))
    register int v0 asm("v0");
    return v0;
}
