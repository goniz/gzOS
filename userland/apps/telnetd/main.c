#include <stdio.h>
#include <libc/socket.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/queue.h>
#include <libc/waitpid.h>
#include <string.h>
#include <libc/traceme.h>
#include <sys/signal.h>
#include <libc/kill.h>
#include <ioctl.h>
#include "vlad.h"
#include "telnet_protocol.h"

typedef union {
    int fds[2];
    struct {
        int read_fd;
        int write_fd;
    } pipe;
} pipe_t;

struct client_ctx {
    TAILQ_ENTRY(client_ctx) next;
    vlad_handle_t vlad;
    int socket_fd;
    int pid;
    bool cleanup;

    struct {
        pipe_t in;
        pipe_t out;
    } pipes;
};

struct server_ctx {
    int server_fd;
    vlad_handle_t vlad;
    TAILQ_HEAD(, client_ctx) clients;
};

static bool telnetd_accept_client(struct server_ctx* telnetd);
static void telnetd_teardown(struct server_ctx* telnetd);
static bool telnetd_add_client(struct server_ctx* telnetd, int fd);
static int telnetd_create_server(void) {
    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (0 > sock) {
        perror("socket failed");
        return -1;
    }

    printf("[telnetd] socket created %d\n", sock);

    const sockaddr_t bindaddr = {
            .address = 0,
            .port = 23
    };

    if (0 != bind(sock, &bindaddr, sizeof(bindaddr))) {
        perror("bind failed");
        close(sock);
        return -1;
    }

    uint8_t* address_as_bytes = (uint8_t*)&bindaddr.address;
    printf("[telnetd] socket bound to %d.%d.%d.%d:%d\n",
           address_as_bytes[0],
           address_as_bytes[1],
           address_as_bytes[2],
           address_as_bytes[3],
           bindaddr.port);

    if (0 != listen(sock, 1)) {
        perror("listen failed");
        close(sock);
        return -1;
    }

    printf("[telnetd] socket is listening\n");

    return sock;
}

static int telnetd_init(struct server_ctx* telnetd) {
    TAILQ_INIT(&telnetd->clients);

    telnetd->vlad = vlad_init();
    if (!telnetd->vlad) {
        printf("vlad init failed\n");
        return 0;
    }

    printf("[telnetd] vlad initialized\n");

    telnetd->server_fd = telnetd_create_server();
    if (0 > telnetd->server_fd) {
        telnetd_teardown(telnetd);
        printf("failed to initialize server\n");
        return 0;
    }

    if (!vlad_register_fd(telnetd->vlad,
                          telnetd->server_fd,
                          (vlad_handler_func)telnetd_accept_client,
                          telnetd))
    {
        telnetd_teardown(telnetd);
        printf("failed to register server fd\n");
        return 0;
    }

    return 1;
}

static void close_fd(int* fd) {
    if (!fd) {
        return;
    }

    if (0 > *fd) {
        return;
    }

    close(*fd);
    *fd = -1;
}

static void telnetd_free_client(struct client_ctx* client) {
    if (!client) {
        return;
    }

    printf("[telnetd] removing client (sock %d, pid %d)\n", client->socket_fd, client->pid);

    vlad_unregister_fd(client->vlad, client->socket_fd);
    vlad_unregister_fd(client->vlad, client->pipes.out.pipe.read_fd);

    close_fd(&client->socket_fd);
    close_fd(&client->pipes.in.pipe.read_fd);
    close_fd(&client->pipes.in.pipe.write_fd);
    close_fd(&client->pipes.out.pipe.read_fd);
    close_fd(&client->pipes.out.pipe.write_fd);

    if (-1 != client->pid) {
        printf("[telnetd] killing pid %d\n", client->pid);
        kill(client->pid, SIGKILL);
        printf("[telnetd] waiting for pid %d\n", client->pid);
        int exitcode = waitpid(client->pid);
        printf("[telnetd] pid %d collected\n", client->pid, exitcode);
    }

    free(client);
}

static void telnetd_teardown(struct server_ctx* telnetd) {

    while (!TAILQ_EMPTY(&telnetd->clients)) {
        struct client_ctx* client = TAILQ_FIRST(&telnetd->clients);
        TAILQ_REMOVE(&telnetd->clients, client, next);

        telnetd_free_client(client);
    }

    vlad_unregister_fd(telnetd->vlad, telnetd->server_fd);
    close_fd(&telnetd->server_fd);

    vlad_free(telnetd->vlad);
}

static bool telnetd_accept_client(struct server_ctx* telnetd) {
    sockaddr_t clientaddr = {0, 0};
    size_t clientaddrlen = sizeof(clientaddr);
    int client = accept(telnetd->server_fd, &clientaddr, &clientaddrlen);
    if (0 > client) {
        perror("accept failed");
        return 0;
    }

    printf("[telnetd] accepted new client! %d\n", client);

    return telnetd_add_client(telnetd, client);
}

static pid_t telnetd_exec(struct client_ctx* client) {

    if (!client) {
        return -1;
    }

    if (0 != pipe(client->pipes.in.fds)) {
        return -1;
    }

    if (0 != pipe(client->pipes.out.fds)) {
        return -1;
    }

    int child_pid = fork();

    // error
    if (-1 == child_pid) {
        return -1;
    }

    // father
    if (0 < child_pid) {
        close_fd(&client->pipes.in.pipe.read_fd);
        close_fd(&client->pipes.out.pipe.write_fd);
        return child_pid;
    }

    // child
    dup2(client->pipes.in.pipe.read_fd, STDIN_FILENO);
    dup2(client->pipes.out.pipe.write_fd, STDOUT_FILENO);
    dup2(client->pipes.out.pipe.write_fd, STDERR_FILENO);

    char* const argv[] = {NULL};
    execv("/bin/shell", argv);
    exit(1);
}

static struct client_ctx* telnetd_alloc_client(struct server_ctx* telnetd) {
    struct client_ctx* client = malloc(sizeof(*client));
    if (!client) {
        return NULL;
    }

    memset(client, 0, sizeof(client));

    client->vlad = telnetd->vlad;
    client->pid = -1;
    client->socket_fd = -1;
    client->cleanup = false;
    client->pipes.in.pipe.read_fd = -1;
    client->pipes.in.pipe.write_fd = -1;
    client->pipes.out.pipe.read_fd = -1;
    client->pipes.out.pipe.write_fd = -1;

    return client;
}

static bool telnetd_read_client_socket(struct client_ctx* client) {
    char buffer[1024];

    int result = recv(client->socket_fd, buffer, sizeof(buffer), 0);
    if (0 >= result) {
        client->cleanup = true;
        return false;
    }

    write(client->pipes.in.pipe.write_fd, buffer, result);
    return true;
}

static bool telnetd_read_client_shell(struct client_ctx* client) {
    char buffer[1024];

    int result = read(client->pipes.out.pipe.read_fd, buffer, sizeof(buffer));
    if (0 >= result) {
        client->cleanup = true;
        return false;
    }

    send(client->socket_fd, buffer, result, 0);
    return true;
}

static bool telnetd_add_client(struct server_ctx* telnetd, int fd) {
    struct client_ctx* client = telnetd_alloc_client(telnetd);
    if (!client) {
        return false;
    }

    client->socket_fd = fd;

//    ioctl(client->socket_fd, FD_SET_BLOCKING, 0);

    if (!vlad_register_fd(client->vlad,
                          client->socket_fd,
                          (vlad_handler_func)telnetd_read_client_socket,
                          client))
    {
        telnetd_free_client(client);
        return false;
    }


    client->pid = telnetd_exec(client);
    if (0 > client->pid) {
        telnetd_free_client(client);
        return false;
    }

    if (!vlad_register_fd(client->vlad,
                          client->pipes.out.pipe.read_fd,
                          (vlad_handler_func)telnetd_read_client_shell,
                          client))
    {
        telnetd_free_client(client);
        return false;
    }

    if (TAILQ_EMPTY(&telnetd->clients)) {
        TAILQ_INSERT_HEAD(&telnetd->clients, client, next);
    } else {
        TAILQ_INSERT_TAIL(&telnetd->clients, client, next);
    }

    return true;
}

static void telnetd_cleanup_clients(struct server_ctx* telnetd)
{
    struct client_ctx* item = NULL;
    struct client_ctx* temp_item = NULL;
    TAILQ_FOREACH_SAFE(item, &telnetd->clients, next, temp_item) {
        if (!item->cleanup) {
            continue;
        }

        TAILQ_REMOVE(&telnetd->clients, item, next);
        telnetd_free_client(item);
    }
}

static void telnetd_do_loop(struct server_ctx* telnetd)
{
    // TODO: add eventfd to kernel
    // TODO: use it to register an eventfd to vlad for control
    // TODO: remove stale clients

    while (true) {
        vlad_loop_once(telnetd->vlad);
        telnetd_cleanup_clients(telnetd);
    }
}

static struct server_ctx telnetd;

int main(int argc, char **argv) {
    puts("telnetd init");

    if (!telnetd_init(&telnetd)) {
        return -1;
    }

    telnetd_do_loop(&telnetd);

    telnetd_teardown(&telnetd);
    return 0;
}
