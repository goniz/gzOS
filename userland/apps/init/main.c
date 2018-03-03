#include <stdio.h>
#include <sys/stat.h>
#include <libc/mount.h>
#include <signal.h>
#include <libc/kill.h>
#include <unistd.h>
#include <stdlib.h>
#include <libc/waitpid.h>

struct inittab {
    const char* filename;
    pid_t pid;
};

struct inittab _inittab[] = {
	{"/bin/telnetd", -1},
//    {"/bin/shell", -1}
};

static void create_dirs() {
    mkdir("/tmp", 0);
    mkdir("/usr", 0);
    mkdir("/pflash", 0);
    mkdir("/proc", 0);
}

static void mount_all() {
    if (0 != mount("procfs", "none", "/proc")) {
        printf("failed to mount /proc\n");
    }

    if (0 != mount("pflashfs", "none", "/pflash")) {
        printf("failed to mount /pflash\n");
    }

    if (0 != mount("fat32", "/pflash/userdata", "/usr")) {
        printf("failed to mount /usr\n");
    }
}

static void execute_inittab() {
    int n_inittab = (sizeof(_inittab) / sizeof(struct inittab));

    printf("inittab jobs: %d\n", n_inittab);

    for (int i = 0; i < n_inittab; i++) {

        printf("job: %s\n", _inittab[i].filename);

        pid_t pid = fork();
        if (0 > pid) {
            printf("fork failed!\n");
            return;
        }

        // father
        if (0 < pid) {
            _inittab[i].pid = pid;
            continue;
        }

        // child
        if (0 == pid) {
            execl(_inittab[i].filename, _inittab[i].filename, NULL);

            printf("exec failed: %s\n", _inittab[i].filename);
            exit(-1);
        }
    }

    for (int i = 0; i < n_inittab; i++) {
        struct inittab* item = &_inittab[i];
        if (-1 == item->pid || 0 == item->pid) {
            continue;
        }

        waitpid(item->pid);
    }
}

int main(int argc, char **argv)
{
    puts("gzOS init started");

    create_dirs();
    mount_all();
    execute_inittab();

    pause();
//    interface_add("eth0", 0x01010101, 0xffffff00);
    
    printf("gzOS init exiting..!\n");
    return 1;
}
