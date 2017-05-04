#include <stdio.h>
#include <unistd.h>
#include <libc/waitpid.h>

void be_a_father(int fds[2]) {
    // close the read end
    close(fds[0]);

    for (int i = 0; i < 5; i++) {
        char buf[64];
        write(fds[1], buf, sprintf(buf, "HI CHILD!! (%d)\n", i));
        sleep(1);
    }

    close(fds[1]);
}

void be_a_child(int fds[2]) {
    // close the write end
    close(fds[1]);

    for (int i = 0; i < 5; i++) {
        char buf[128] = {0};
        read(fds[0], buf, sizeof(buf));
        printf("buf: %s\n", buf);
    }

    close(fds[0]);
}

int main(int argc, char **argv) {
    puts("Running pipe tests..");

    int result;
    int fds[2];

    result = pipe(fds);
    if (0 != result) {
        printf("pipe() failed\n");
        return -1;
    }

    result = fork();
    if (result > 0) {
        printf("father returned, child is %d\n", result);
        be_a_father(fds);
        result = waitpid(result);
        printf("child exited with %d\n", result);
    } else {
        printf("child returned, my pid is %d\n", getpid());
        be_a_child(fds);
    }

    return 0;
}
