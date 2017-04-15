#include <stdio.h>
#include <libc/socket.h>
#include <unistd.h>
#include <libc/traceme.h>

int main(int argc, char **argv) {
    puts("telnetd init");

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

    sockaddr_t clientaddr = {0, 0};
    size_t clientaddrlen = sizeof(clientaddr);
    int client = accept(sock, &clientaddr, &clientaddrlen);
    if (0 > client) {
        perror("accept failed");
        close(sock);
        return -1;
    }

    printf("[telnetd] accepted new client! %d\n", client);

//    traceme(1);

    while (1)
    {
        char buf[512] = {0};
        int ret = recv(client, buf, sizeof(buf), 0);
        printf("recv: %d\n", ret);
        if (-1 == ret) {
            close(client);
            close(sock);
            return 1;
        }

        if (0 == ret) {
            break;
        }

        if (-1 == send(client, buf, (size_t) ret, 0)) {
            close(sock);
            close(client);
            return 1;
        }
    }

    close(client);
    close(sock);
    return 0;
}
