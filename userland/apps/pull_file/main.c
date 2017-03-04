#include <stdio.h>
#include <libc/socket.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

int main(int argc, char** argv)
{
    uint8_t buf[512];

    printf("argc = %d\n", argc);
    for (int i = 0; i < argc; i++) {
        printf("argv[%d] = %s\n", i, argv[i]);
    }

    if (argc != 3) {
        return -1;
    }

    uint32_t ip1, ip2, ip3, ip4;
    uint32_t port;

    int result = sscanf(argv[1], "%d.%d.%d.%d:%d", &ip1, &ip2, &ip3, &ip4, &port);
    printf("result: %d\n", result);

    if (5 != result) {
        return -1;
    }

    uint32_t ip = (ip1 << 24) | (ip2 << 16) | (ip3 << 8) | ip4;
    sockaddr_t addr = {ip, port};
    int ret = 0;

    printf("ip: %08x, port: %d\n", ip, port);

    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (0 > sock) {
        printf("failed to create socket\n");
        return -1;
    }

    if (0 != connect(sock, &addr)) {
        printf("failed to connect\n");
        close(sock);
        return -1;
    }

    printf("writing to %s\n", argv[2]);

    int file = open(argv[2], O_WRONLY | O_CREAT);
    if (-1 == file) {
        printf("failed to open file\n");
        close(sock);
        return -1;
    }

    while (1) {
        memset(buf, 0, sizeof(buf));
        result = recv(sock, buf, 16, 0);
        if (0 == result) {
            printf("socket closed by remote host\n");
            break;
        } else if (-1 == result) {
            printf("an error occurred in recv\n");
            goto exit;
        }

        write(file, buf, result);
    }

exit:
    close(sock);
    close(file);
    return ret;
}
