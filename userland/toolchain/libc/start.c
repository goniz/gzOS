
#include <stdlib.h>
#include <string.h>

extern int __init_array_start;
extern int __init_array_end;
extern char __bss;
extern char __ebss;
extern int main(int argc, const char* argv[]);

static void clear_bss(void)
{
    char* start = &__bss;
    char* end = &__ebss;

    memset(start, 0, end - start);
}

static void invoke_constructors(void)
{
    typedef void (*func_t)(void);
    int* init_array = &__init_array_start;
    int* init_array_end = &__init_array_end;

    while (init_array < init_array_end) {
        func_t func = (func_t)(*init_array++);
        func();
    }
}

void _start(void* argument)
{
    const char* argv[] = {};

    clear_bss();
    invoke_constructors();

    exit(main(0, argv));
    while(1);
}