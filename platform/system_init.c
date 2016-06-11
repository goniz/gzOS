#include <stdio.h>

static void initialize_stdio(void);
static void invoke_constructors(void);

extern int __init_array_start;
extern int __init_array_end;

extern int main(int argc, char **argv, char **envp);
void system_init(int argc, char **argv, char **envp)
{
	initialize_stdio();
	invoke_constructors();

    /* We should never return from main ... */
    main(argc, argv, envp);

    /* ... but if we do, safely trap here */
	printf("main function exited. hanging.\n");
    while(1)
    {
        /* EMPTY! */
    }
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
        printf("Invoking ctor at %p\r\n", func);
        func();
    }
}

