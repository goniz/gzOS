#include <platform/kprintf.h>

__attribute__((noreturn))
void panic(const char* fmt, ...)
{
    va_list arg;

    va_start(arg, fmt);
    kprintf("[panic] %s:%d: ", __FILE__, __LINE__);
	vkprintf(fmt, arg);
    kprintf("\n");
    va_end(arg);

	while (1);
}
