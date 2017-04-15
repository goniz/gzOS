#include <stdio.h>
#include <sys/stat.h>
#include <libc/mount.h>
#include <signal.h>
#include <libc/kill.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    puts("gzOS init started");

    mkdir("/tmp", 0);

    mkdir("/proc", 0);
    if (0 != mount("procfs", "none", "/proc")) {
        printf("failed to mount /proc\n");
    }

//    mkdir("/dev", 0);
//    if (0 != mount("devfs", "none", "/dev")) {
//        printf("failed to mount /dev\n");
//    }

    mkdir("/pflash", 0);
    if (0 != mount("pflashfs", "none", "/pflash")) {
        printf("failed to mount /pflash\n");
    }

    mkdir("/usr", 0);
    if (0 != mount("fat32", "/pflash/userdata", "/usr")) {
        printf("failed to mount /usr\n");
    }

    execl("/bin/shell", "shell", NULL);
    execl("/bin/telnetd", "telnetd", NULL);

    pause();
//    interface_add("eth0", 0x01010101, 0xffffff00);

    return 1;
}
