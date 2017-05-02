
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

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

__unused
void _start(void* argument)
{

    int argc = 0;
    const char** argv = argument;
    const char** pos = argument;
    while (NULL != *pos++) {
        argc++;
    }

    clear_bss();

    _REENT_INIT_PTR(_REENT);

    invoke_constructors();

    int exitcode = main(argc, argv);

    exit(exitcode);
}