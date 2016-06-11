//
// Created by gz on 6/11/16.
//

#include <stdarg.h>
#include <platform/kprintf.h>

void kprintf(const char* fmt, ...)
{
    va_list arg;

    va_start(arg, fmt);
    vkprintf(fmt, arg);
    va_end(arg);
}
