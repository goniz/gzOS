
#include <syscall.h>
#include <stdarg.h>
#include <stddef.h>

static int count_args(const char* argv0, va_list args) {
    int count = 1;

    while (NULL != va_arg(args, const char*)) {
        count += 1;
    }

    return count;
}

int execl(const char* path, const char* argv0, ...)
{
    va_list args;
    va_start(args, argv0);
    int argc = count_args(argv0, args);
    const char* argv[argc];

    argv[0] = argv0;
    for (int i = 1; i < argc; i++) {
        argv[i] = va_arg(args, const char*);
    }
    va_end(args);

    return syscall(SYS_NR_EXEC, path, argc, argv);
}

int execv(const char* path, char * const argv[])
{
    int argc = 0;
    for (; NULL != argv[argc]; argc++) {}

    return syscall(SYS_NR_EXEC, path, argc, argv);
}