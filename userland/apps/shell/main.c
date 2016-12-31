#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <stdlib.h>
#include <libc/readdir.h>
#include <libc/traceme.h>
#include <string.h>


static int ls_main(int argc, char* argv[]) {
    struct DirEntry dirent;
    int ret;

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

    for (int index = optind; index < argc; index++) {
//        printf("Non-option argument %s\n", argv[index]);

        int readdirfd = readdir_create(argv[index]);
        if (-1 == readdirfd) {
            printf("failed to create readdir handle\n");
            return 0;
        }

        while (0 < readdir_read(readdirfd, &dirent)) {
            printf("* %s%s\n", dirent.name, dirent.type == DIRENT_DIR ? "/" : "");
        }

        readdir_close(readdirfd);
    }

    return 0;
}

static int exec_main(int argc, char* argv[]) {
    if (argc <= 1) {
        return -1;
    }

    return execv(argv[1], argv + 2);
}

static int trace_main(int argc, char* argv[]) {
    if (argc != 2 || NULL == argv[1]) {
        return -1;
    }

    int state = 0;
    if (0 == strcmp(argv[1], "on")) {
        state = 1;
    } else if (0 == strcmp(argv[1], "off")) {
        state = 0;
    }

    traceme(state);
    return 0;
}

static int exit_main(int argc, char* argv[]) {
    return 1;
}

struct cmdline {
    const char* cmd;
    int (*cmd_main)(int argc, char** argv);
};

static struct cmdline _commands[] = {
        {"ls", ls_main},
        {"trace", trace_main},
        {"exec", exec_main},
        {"exit", exit_main},
        {"q", exit_main}
};

static void prompt(void) {
    printf("gzOS# ");
    fflush(stdout);
}

static int count_spaces(const char* line) {
    int spaces;
    for (spaces = 0; line[spaces]; line[spaces] == ' ' ? spaces++ : *line++);

    return spaces;
}

static int handle_line(char* line) {
    int argc = count_spaces(line)+ 1;
    char* argv[argc];

    memset(argv, 0, sizeof(argv));

    int index = 0;
    const char* token = NULL;
    while (NULL != (token = strsep(&line, " "))) {
        argv[index] = (char *) token;
        index++;
    }

//    printf("argc: %d\n", argc);
//    for (int i = 0; i < argc; i++) {
//        printf("argv[%d] = %p\n", i, (void *) argv[i]);
//    }

    for (int i = 0; i < (sizeof(_commands) / sizeof(_commands[0])); i++) {
        struct cmdline* cmd = &_commands[i];

//        printf("cmd: %s\n", cmd->cmd);

        if (0 == strcmp(cmd->cmd, argv[0])) {
            return cmd->cmd_main(argc, argv);
        }
    }

    return 0;
}

static void output_char(int c) {
    fputc(c, stdout);
    fflush(stdout);
}

int main(int argc, char **argv) {
    char linebuf[128];
    int index = 0;
    int c = 0;
    memset(linebuf, 0, sizeof(linebuf));

//    traceme(1);

    prompt();

    while (EOF != (c = fgetc(stdin))) {
        if ('\n' == c) {
            output_char(c);

            if (0 == index) {
                prompt();
                continue;
            }

            linebuf[index] = '\0';

            int result = handle_line(linebuf);
            switch (result) {
                case 0:
                    break;

                case 1:
                    printf("quiting..\n");
                    return 0;

                default:
                    printf("exit code: %d\n", result);
                    break;
            }

            index = 0;
            memset(linebuf, 0, sizeof(linebuf));

            prompt();
            continue;
        }

        // backspace
        if (0x7f == c) {
            linebuf[index] = '\0';
            if (0 != index) {
                index--;
            }

            output_char('\b');
            continue;
        }

        output_char(c);

        linebuf[index] = (char) c;
        index++;
        if (index >= sizeof(linebuf)) {
            index = 0;
        }
    }

    return 0;
}
