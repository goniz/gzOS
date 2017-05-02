#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <libc/traceme.h>
#include <libc/waitpid.h>
#include <alloca.h>

static int exec_main(int argc, char** argv) {
    char** cmd_argv = argv;
    int cmd_argc = argc;

    char** exec_argv = alloca(sizeof(char*) * (cmd_argc + 1)); // +1 for the null at the end
    for (int i = 0; i < cmd_argc; i++) {
        exec_argv[i] = cmd_argv[i];
    }
    exec_argv[cmd_argc] = NULL;

    pid_t newpid = execv(*exec_argv, exec_argv);
    if (-1 == newpid) {
        return -1;
    }

    printf("new pid: %d\n", newpid);

    int exit_code = waitpid(newpid);

    printf("exit code: %d\n", exit_code);

    return 0;
}

int main(int argc, char **argv) {
    puts("Running fork tests..");

    int result = fork();

    if (result > 0) {
        printf("father returned, child is %d\n", result);
        result = waitpid(result);
        printf("child exited with %d\n", result);
    } else {
        printf("child returned, my pid is %d\n", getpid());
    }

    return 0;
}
