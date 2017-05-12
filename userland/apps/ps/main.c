#include <stdio.h>
#include <libc/readdir.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

static void print_title(void) {
    printf("PID\tCPU\tCWD\tCMD\t\n");
}

static int read_file(const char* path, char* buf, size_t size) {
    FILE* fp = fopen(path, "rb");
    if (!fp) {
        return -1;
    }

    int result = fread(buf, 1, size, fp);

    fclose(fp);

    return result;
}

static int read_proc_file(const char* pid, const char* filename, char* buf, size_t size) {
    char path[64] = {0};

    snprintf(path, sizeof(path) - 1, "/proc/%s/%s", pid, filename);

    return read_file(path, buf, size);
}

static void print_pid(const char* pid) {
    char name[64] = {0};
    char cpu[64] = {0};
    char cwd[64] = {0};
    char cmdline[128] = {0};

    read_proc_file(pid, "name", name, sizeof(name));
    read_proc_file(pid, "cmdline", cmdline, sizeof(cmdline));
    read_proc_file(pid, "cpu", cpu, sizeof(cpu));
    read_proc_file(pid, "cwd", cwd, sizeof(cwd));

    printf("%s\t", pid);
    printf("%s\t", cpu);
    printf("%s\t", cwd);
    printf("%s\t", 0 == strcmp("", cmdline) ? name : cmdline);
    printf("\n");
}

int main(int argc, char** argv) {

    int delay = -1;
    if (2 == argc) {
        delay = atoi(argv[1]);
    }

    do {

        int proc_fd = readdir_create("/proc");
        if (0 > proc_fd) {
            printf("failed to readdir /proc\n");
            return -1;
        }

        if (0 < delay) {
            printf("\033[2J\r");
        }

        print_title();

        struct DirEntry dirent;
        while (0 < readdir_read(proc_fd, &dirent)) {
            print_pid(dirent.name);
        }

        readdir_close(proc_fd);

        if (0 < delay) {
            sleep(delay);
        } else {
            break;
        }

    } while (1);

    return 0;
}
