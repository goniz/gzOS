#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <stdlib.h>
#include <libc/readdir.h>
#include <libc/traceme.h>


static void ls(const char* path)
{
    struct DirEntry dirent;
    int ret;
    int readdirfd = readdir_create(path);
    if (-1 == readdirfd) {
        printf("failed to create readdir handle\n");
        return;
    }

    printf("ls %s\n", path);
    while (0 < (ret = readdir_read(readdirfd, &dirent))) {
        printf("* %s%s\n", dirent.name, dirent.type == DIRENT_DIR ? "/" : "");
    }

    printf("ret: %d\n", ret);
    readdir_close(readdirfd);
}

int main(int argc, char **argv) {
//    traceme(1);

    puts("Hello World 1!");

    int list_flag = 0;
    int human_flag = 0;
    int c;

    opterr = 0;
    while ((c = getopt(argc, argv, "lh")) != -1) {
        switch (c) {
            case 'l':
                list_flag = 1;
                break;
            case 'h':
                human_flag = 1;
                break;
            case '?':
                if (isprint (optopt))
                    fprintf(stderr, "Unknown option `-%c'.\n", optopt);
                else
                    fprintf(stderr,
                            "Unknown option character `\\x%x'.\n",
                            optopt);
                return 1;

            default:
                return 1;
        }
    }

    for (int index = optind; index < argc; index++)
        printf("Non-option argument %s\n", argv[index]);

    return 1;
}
