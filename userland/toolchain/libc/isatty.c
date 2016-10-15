#include <unistd.h>
#include <errno.h>

/*
 isatty
 Query whether output stream is a terminal. For consistency with the other minimal implementations,
 */
int isatty(int file) {
    switch (file) {
        case STDOUT_FILENO:
        case STDERR_FILENO:
        case STDIN_FILENO:
            return 1;
        default:
            //errno = ENOTTY;
            errno = EBADF;
            return 0;
    }
}