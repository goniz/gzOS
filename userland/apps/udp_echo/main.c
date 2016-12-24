#include <stdio.h>
#include <libc/socket.h>
#include <unistd.h>

int main(int argc, char** argv)
{
    puts("Hello UDP!");

    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (-1 == sock) {
        printf("failed to create socket\n");
        return 1;
    }

    sockaddr_t sockaddr;
    sockaddr.address = 0;
    sockaddr.port = 9999;
    if (0 != bind(sock, &sockaddr, sizeof(sockaddr))) {
        printf("failed to bind\n");
        close(sock);
        return 1;
    }

    while (1)
    {
        char buf[512] = {0};
        sockaddr_t addr = {0};
        int ret = recvfrom(sock, buf, sizeof(buf), 0, &addr, NULL);
        printf("recvfrom: %d\n", ret);
        if (-1 == ret) {
            close(sock);
            return 1;
        }

        if (0 == ret) {
            close(sock);
            break;
        }

        if (-1 == sendto(sock, buf, (size_t) ret, 0, &addr, sizeof(addr))) {
            close(sock);
            return 1;
        }
    }

    return 0;
}
