/*
 * newlib_stubs.c
 *
 *  Created on: 2 Nov 2010
 *      Author: nanoage.co.uk
 */
#include <errno.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <sys/unistd.h>
#include <string>
#include <platform/panic.h>
#include <lib/syscall/syscall.h>
#include <sys/time.h>
#include <platform/clock.h>

extern "C" {

#undef errno
extern int errno;

extern caddr_t _get_stack_pointer(void);

/*
 environ
 A pointer to a list of environment variables and their values.
 For a minimal environment, this empty list is adequate:
 */
char *__env[1] = {0};
char **environ = __env;

__attribute__((noreturn))
void _exit(int status) {
	panic("_exit called.");
}

int _open_r(struct _reent * reent, const char *pathname, int flags, mode_t mode)
{
	errno = -ENOENT;
	return 0;
}

int _close_r(int file)
{
	errno = -ENOENT;
	return -1;
}

/*
 execve
 Transfer control to a new process. Minimal implementation (for a system without processes):
 */
int _execve(char *name, char **argv, char **env) {
    errno = ENOMEM;
    return -1;
}
/*
 fork
 Create a new process. Minimal implementation (for a system without processes):
 */

int _fork() {
    errno = EAGAIN;
    return -1;
}
/*
 fstat
 Status of an open file. For consistency with other minimal implementations in these examples,
 all files are regarded as character special devices.
 The `sys/stat.h' header file required is distributed in the `include' subdirectory for this C library.
 */
int _fstat_r(struct _reent * reent, int file, struct stat *st) {
    st->st_mode = S_IFCHR;
    return 0;
}

/*
 getpid
 Process-ID; this is sometimes used to generate strings unlikely to conflict with other processes. Minimal implementation, for a system without processes:
 */
pid_t getpid(void)
{
    return syscall(SYS_NR_GET_PID);
}

/*
 isatty
 Query whether output stream is a terminal. For consistency with the other minimal implementations,
 */
int _isatty_r(struct _reent * reent, int file) {
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


/*
 kill
 Send a signal. Minimal implementation:
 */
int kill(int pid, int sig)
{
//    kprintf("sending %d to %d\n", sig, pid);
    return syscall(SYS_NR_SIGNAL, pid, sig);
}

/*
 link
 Establish a new _name for an existing file. Minimal implementation:
 */

int _link(char *oldName, char * newName) {
    errno = EMLINK;
    return - 1 ;
}

/*
 lseek
 Set position in a file. Minimal implementation:
 */
int _lseek_r(struct _reent * reent, int file, int ptr, int dir) {
    return 0;
}

/*
 read
 Read a character to a file. `libc' subroutines will use this system routine for input from all files, including stdin
 Returns -1 on error or blocks until the number of characters have been read.
 */
extern "C" int uart_read(char *str, size_t n);
int read(int file, void* ptr, size_t len)
{
    return uart_read((char *) ptr, len);
}

/*
 stat
 Status of a file (by _name). Minimal implementation:
 int    _EXFUN(stat,( const char *__path, struct stat *__sbuf ));
 */

int _stat_r(struct _reent * reent, const char *filepath, struct stat *st) {
    st->st_mode = S_IFCHR;
    return 0;
}

/*
 times
 Timing information for current process. Minimal implementation:
 */

clock_t _times(struct tms *buf) {
    return (clock_t) -1;
}

/*
 unlink
 Remove a file's directory entry. Minimal implementation:
 */
int unlink(const char *name) {
    errno = ENOENT;
    return -1;
}

/*
 write
 Write a character to a file. `libc' subroutines will use this system routine for output to all files, including stdout
 Returns -1 on error or number of bytes sent
 */
extern "C" int uart_write(const char* s, size_t n);

extern "C"
int write(int file, const void* buf, size_t len)
{
    uart_write((const char *) buf, (size_t) len);
    return (int) len;
}

extern "C"
int gettimeofday(struct timeval* tv, void* tz)
{
    if (tz) {
        ((struct timezone*)tz)->tz_dsttime = 0;
        ((struct timezone*)tz)->tz_minuteswest = 0;
    }

    if (tv) {
        uint64_t now = clock_get_ms();
        tv->tv_sec = (time_t) (now / 1000);
        tv->tv_usec = (suseconds_t) (now % 1000);
    }

    return 0;
}

} // extern "C"
