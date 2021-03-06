#include <getopt.h>
#include <stdio.h>
#include <unistd.h>
#include <libc/readdir.h>
#include <libc/traceme.h>
#include <string.h>
#include <stdint.h>
#include <libc/socket.h>
#include <libc/endian.h>
#include <libc/waitpid.h>
#include <alloca.h>
#include <ctype.h>
#include <stdlib.h>

static int eval_line(char* line);

static int cd_main(int argc, char* argv[]) {
    if (argc != 2) {
        return -1;
    }

    return chdir(argv[1]);
}

static int pwd_main(int argc, char* argv[]) {
    char buf[128];

    if (0 != getcwd(buf, sizeof(buf))) {
        printf("getcwd failed\n");
        return -1;
    }

    puts(buf);
    return 0;
}

static void ls_dir(const char* path) {
    struct DirEntry dirent;

    int readdirfd = readdir_create(path);
    if (-1 == readdirfd) {
        printf("failed to create readdir handle\n");
        return;
    }

    while (0 < readdir_read(readdirfd, &dirent)) {
        printf("* %s%s - %d bytes\n", dirent.name, dirent.type == DIRENT_DIR ? "/" : "", dirent.size);
    }

    readdir_close(readdirfd);
}

static int ls_main(int argc, char* argv[]) {
    int list_flag = 0;
    int human_flag = 0;
    int c;

    optind = 0;
    opterr = 0;
    optarg = NULL;
    optopt = 0;
    while ((c = getopt(argc, argv, "+lh")) != -1) {
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
                return -1;

            default:
                return -1;
        }
    }

    if (optind >= argc) {
        ls_dir(".");
    }

    for (int index = optind; index < argc; index++) {
        printf("Non-option argument %s\n", argv[index]);

        ls_dir(argv[index]);
    }

    return 0;
}

/* sample: exec /usr/BIN/LS /proc
 * argc => 3
 * argv[0] = exec
 * argv[1] = /usr/BIN/LS
 * argv[2] = /proc
 * */
static int exec_main(int argc, char* argv[]) {
    if (argc <= 1) {
        return -1;
    }

    char** cmd_argv = argv + 1;
    int cmd_argc = argc - 1;

    char** exec_argv = alloca(sizeof(char*) * (cmd_argc + 1)); // +1 for the null at the end
    for (int i = 0; i < cmd_argc; i++) {
        exec_argv[i] = cmd_argv[i];
    }
    exec_argv[cmd_argc] = NULL;

    pid_t newpid = execv(*exec_argv, exec_argv);
    if (-1 == newpid) {
        printf("execv failed!\n");
        return -1;
    }

    return 0;
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

static int connect_main(int argc, char* argv[]) {
    return eval_line("exec /usr/BIN/NC 1.1.1.2:8888");
}

static int cat_main(int argc, char* argv[]) {
    if (2 != argc) {
        return -1;
    }

    FILE* fp = fopen(argv[1], "rb");
    if (!fp) {
        return -1;
    }

    do {
        char buffer[1024];
        int bytes = fread(buffer, 1, sizeof(buffer), fp);
        fwrite(buffer, 1, bytes, stdout);

    } while (!feof(fp) && !ferror(fp));

    fclose(fp);
    return 0;
}

static int sleep_main(int argc, char* argv[]) {
    if (2 != argc) {
        return -1;
    }

    sleep(atoi(argv[1]));
    return 0;
}

static int exit_main(int argc, char* argv[]) {
    return 1;
}

static int do_exec(int argc, char* argv[]) {
    // prepare args
    char** exec_argv = alloca(sizeof(char*) * (argc + 1)); // +1 for the null at the end
    for (int i = 0; i < argc; i++) {
        exec_argv[i] = argv[i];
    }
    exec_argv[argc] = NULL;

    // fork and exec
    pid_t pid = fork();
    if (0 > pid) {
        printf("fork failed!\n");
        return -1;
    }

    // father
    if (0 < pid) {
        return waitpid(pid);
    }

    // child
    if (0 == pid) {
        execv(*exec_argv, exec_argv);

        printf("%s: no such file or directory\n", *exec_argv);
        exit(-1);
    }
}

struct cmdline {
    const char* cmd;
    int (*cmd_main)(int argc, char** argv);
};

static struct cmdline _commands[] = {
        {"cd", cd_main},
        {"pwd", pwd_main},
        {"ls", ls_main},
        {"trace", trace_main},
        {"exec", exec_main},
        {"connect", connect_main},
        {"cat", cat_main},
	    {"sleep", sleep_main},
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

static int eval_line(char* line) {
    int argc = count_spaces(line) + 1;
    char* argv[argc];

    memset(argv, 0, sizeof(argv));

    int index = 0;
    const char* token = NULL;
    while (NULL != (token = strsep(&line, " "))) {
        argv[index] = (char *) token;
        index++;
    }

    for (int i = 0; i < (sizeof(_commands) / sizeof(_commands[0])); i++) {
        struct cmdline* cmd = &_commands[i];

//        printf("cmd: %s\n", cmd->cmd);

        if (0 == strcmp(cmd->cmd, argv[0])) {
            return cmd->cmd_main(argc, argv);
        }
    }

    return do_exec(argc, argv);
}

static void output_char(int c) {
    fputc(c, stdout);
    fflush(stdout);
}

static int parse_ip_port(int argc, char** argv, uint32_t* out_ip, uint16_t* out_port) {
    if (argc != 2) {
        return 0;
    }

    uint32_t ip1, ip2, ip3, ip4;
    uint32_t port;

    int result = sscanf(argv[1], "%d.%d.%d.%d:%d", &ip1, &ip2, &ip3, &ip4, &port);
    if (5 != result) {
        return 0;
    }

    uint32_t ip = (ip1 << 24) | (ip2 << 16) | (ip3 << 8) | ip4;

    if (out_ip) {
        *out_ip = ip;
    }

    if (out_port) {
        *out_port = port;
    }

    return 1;
}

static int connect_and_dup(uint32_t ip, uint16_t port) {
    sockaddr_t addr = {
        .address = htonl(ip),
        .port = htons(port)
    };

    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (0 > sock) {
        printf("failed to create socket\n");
        return 0;
    }

    if (0 != connect(sock, &addr)) {
        printf("failed to connect\n");
        close(sock);
        return 0;
    }

    dup2(sock, STDIN_FILENO);
    dup2(sock, STDOUT_FILENO);
    dup2(sock, STDERR_FILENO);

    close(sock);

    return 1;
}

#define HISTORY_SIZE (10)

static char* current_line = NULL;
static char* history[HISTORY_SIZE] = {0};
static int history_index = 0;

static void history_push(char* line) {
    history[history_index] = strdup(line);
    history_index = ((history_index + 1) % HISTORY_SIZE);
}

int main(int argc, char **argv) {
    int index = 0;
    int c = 0;

    current_line = malloc(128);
    if (!current_line) {
        return -1;
    }

    memset(current_line, 0, sizeof(128));
    memset(history, 0, sizeof(history));

//    traceme(1);
    setvbuf(stdout, NULL, _IONBF, 0);
    int stdin_echo = 1;

    uint32_t ip = 0;
    uint16_t port = 0;
    if (parse_ip_port(argc, argv, &ip, &port) && connect_and_dup(ip, port)) {
        stdin_echo = 0;
    }

    prompt();

    while (EOF != (c = fgetc(stdin))) {
        if ('\n' == c) {
            if (stdin_echo) {
                output_char(c);
            }

            if (0 == index) {
                prompt();
                continue;
            }

            current_line[index] = '\0';

            history_push(current_line);

            int result = eval_line(current_line);
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
            memset(current_line, 0, 128);

            prompt();
            continue;
        }

        // backspace
        if (0x7f == c) {
            current_line[index] = '\0';
            if (0 != index) {
                index--;
            }

            output_char('\b');
            continue;
        }

        // arrows
        if ('\033' == c) {
            fgetc(stdin); // skip the [
            switch(fgetc(stdin)) { // the real value
                case 'A':
                    // code for arrow up
                    if (0 == history_index) {
                        break;
                    }

                    output_char('\r');
                    prompt();
                    fputs(history[history_index - 1], stdout);
                    strncpy(current_line, history[history_index - 1], 128);
                    index = strlen(current_line);

                    break;
                case 'B':
                    // code for arrow down
                    break;
                case 'C':
                    // code for arrow right
                    break;
                case 'D':
                    // code for arrow left
                    break;
            }

            continue;
        }

        if (stdin_echo) {
            output_char(c);
        }

        current_line[index] = (char) c;
        index++;
        if (index >= 128) {
            index = 0;
        }
    }

    return 0;
}
