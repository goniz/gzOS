#include <lib/syscall/syscall.h>
#include "lib/network/socket.h"

DEFINE_SYSCALL(SOCKET, socket, SYS_IRQ_DISABLED) {
    SYSCALL_ARG(int, domain);
    SYSCALL_ARG(int, type);
    SYSCALL_ARG(int, proto);

    return socket_create(domain, type, proto);
}

DEFINE_SYSCALL(BIND, bind, SYS_IRQ_DISABLED) {
    SYSCALL_ARG(int, fd);
    SYSCALL_ARG(const SocketAddress*, address);

    SocketFileDescriptor *sockFd = socket_num_to_fd(fd);
    if (nullptr == sockFd) {
        return -1;
    }

    return sockFd->bind(*address);
}

DEFINE_SYSCALL(LISTEN, listen, SYS_IRQ_DISABLED) {
    SYSCALL_ARG(int, fd);
    SYSCALL_ARG(int, backlog);

    SocketFileDescriptor *sockFd = socket_num_to_fd(fd);
    if (nullptr == sockFd) {
        return -1;
    }

    return sockFd->listen(backlog);
}

DEFINE_SYSCALL(ACCEPT, accept, SYS_IRQ_DISABLED) {
    SYSCALL_ARG(int, fd);
    SYSCALL_ARG(SocketAddress*, address);
    SYSCALL_ARG(size_t*, addr_len);

    SocketFileDescriptor *sockFd = socket_num_to_fd(fd);
    if (nullptr == sockFd) {
        return -1;
    }

    return sockFd->accept(address, addr_len);
}

DEFINE_SYSCALL(CONNECT, connect, SYS_IRQ_ENABLED) {
    SYSCALL_ARG(int, fd);
    SYSCALL_ARG(const SocketAddress*, address);

    SocketFileDescriptor *sockFd = socket_num_to_fd(fd);
    if (nullptr == sockFd) {
        return -1;
    }

    return sockFd->connect(*address);
}

DEFINE_SYSCALL(RECVFROM, recvfrom, SYS_IRQ_ENABLED) {
    SYSCALL_ARG(int, fd);
    SYSCALL_ARG(void*, buf);
    SYSCALL_ARG(size_t, size);
    SYSCALL_ARG(SocketAddress*, address);

    SocketFileDescriptor *sockFd = socket_num_to_fd(fd);
    if (nullptr == sockFd) {
        return -1;
    }

    return sockFd->recvfrom(buf, size, address);
}

DEFINE_SYSCALL(SENDTO, sendto, SYS_IRQ_DISABLED) {
    SYSCALL_ARG(int, fd);
    SYSCALL_ARG(const void*, buf);
    SYSCALL_ARG(size_t, size);
    SYSCALL_ARG(const SocketAddress*, address);

    if (nullptr == buf || nullptr == address) {
        return -1;
    }

    SocketFileDescriptor *sockFd = socket_num_to_fd(fd);
    if (nullptr == sockFd) {
        return -1;
    }

    return sockFd->sendto(buf, size, *address);
}