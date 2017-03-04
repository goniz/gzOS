#include <stdio.h>
#include <libc/socket.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char** argv)
{
    printf("argc = %d\n", argc);

    if (argc != 2) {
        return -1;
    }

    printf("argv[1] = %s\n", argv[1]);

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

    uint8_t buf[512];

    while (1) {
        memset(buf, 0, sizeof(buf));
        result = recv(sock, buf, 16, 0);
        if (0 == result) {
            printf("socket closed by remote host\n");
            goto exit;
        } else if (-1 == result) {
            printf("an error occurred in recv\n");
            goto exit;
        }

        for (int i = 0; i < result; i++) {
            printf("%02x ", buf[i]);
        }
        puts("");

        result = send(sock, buf, result, 0);
        if (0 == result) {
            printf("socket closed by remote host\n");
            goto exit;
        } else if (-1 == result) {
            printf("an error occurred in recv\n");
            goto exit;
        }
    }

exit:
    close(sock);
    return ret;
}
