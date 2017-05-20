#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <libc/traceme.h>
#include <libc/waitpid.h>
#include <alloca.h>

void fork_once() {
    int result = fork();
    if (result > 0) {
        printf("father returned, child is %d\n", result);
        result = waitpid(result);
        printf("child exited with %d\n", result);
    } else {
        printf("child returned, my pid is %d\n", getpid());
        exit(0);
    }

}

int main(int argc, char **argv) {
    puts("Running fork tests..");

    for (int i = 0; i < 5; i++) {
        fork_once();
        sleep(1);
    }

    return 0;
}
