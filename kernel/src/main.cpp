#include <stdio.h>
#include <lib/kernel/proc/Scheduler.h>
#include <ctime>
#include <lib/network/arp/arp.h>
#include <lib/network/interface.h>
#include <lib/network/socket.h>
#include <platform/cpu.h>
#include <lib/network/checksum.h>
#include <platform/malta/mips.h>
#include <platform/malta/malta.h>
#include <lib/kernel/vfs/vfs_api.h>
#include <lib/kernel/initrd/load_initrd.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
extern "C" char _end;
extern "C" caddr_t _get_stack_pointer(void);

std::vector<struct ps_ent> getProcessList(int expected_entries) {
    std::vector<struct ps_ent> buffer((unsigned long) expected_entries);

    int nr_entries = syscall(SYS_NR_PS, buffer.data(), buffer.size() * sizeof(struct ps_ent));
    if (-1 == nr_entries) {
        return {};
    }

    buffer.resize((unsigned long) nr_entries);
    return std::move(buffer);
}

static uint32_t read_size(int sock) {
    uint32_t size = 0;
    SocketAddress clientaddr{};
    if (sizeof(size) != syscall(SYS_NR_RECVFROM, sock, &size, sizeof(size), &clientaddr)) {
        size = 0;
        goto exit;
    }

    size = ntohl(size);

    kprintf("incoming buffer size: %d\n", size);
    if (1 * 1024 * 1024 < size) {
        kprintf("buffer size too big..\n");
        size = 0;
    }

    exit:
    if (clientaddr.address != 0) {
        uint32_t networkSize = htonl(size);
        syscall(SYS_NR_SENDTO, sock, &networkSize, sizeof(networkSize), &clientaddr);
    }
    return size;
}

uint8_t* recv_file_over_udp(size_t* size) {
    if (size) {
        *size = 0;
    }

    int sock = syscall(SYS_NR_SOCKET, AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    printf("sock: %d\n", sock);
    if (-1 == sock) {
        return nullptr;
    }

    SocketAddress addr{IPADDR_ANY, 1234};
    if (-1 == syscall(SYS_NR_BIND, sock, &addr)) {
        syscall(SYS_NR_CLOSE, sock);
        return nullptr;
    }

    uint32_t bufferSize = read_size(sock);
    if (0 == bufferSize) {
        syscall(SYS_NR_CLOSE, sock);
        return nullptr;
    }

    uint8_t* buffer = (uint8_t*) malloc(bufferSize);
    if (!buffer) {
        syscall(SYS_NR_CLOSE, sock);
        return nullptr;
    }

    uint32_t pos = 0;
    while (pos < bufferSize) {
        SocketAddress clientaddr{};
        int ret = syscall(SYS_NR_RECVFROM, sock, buffer + pos, bufferSize - pos, &clientaddr);
        kprintf("read: %d from %08x:%d\n", ret, clientaddr.address, clientaddr.port);
        if (-1 == ret) {
            syscall(SYS_NR_CLOSE, sock);
            free(buffer);
            return nullptr;
        }

        kprintf("acking for %d\n", pos);
        uint32_t networkPos = htonl(pos);
        syscall(SYS_NR_SENDTO, sock, &networkPos, sizeof(networkPos), &clientaddr);

        pos += ret;
    }

    syscall(SYS_NR_CLOSE, sock);

    if (size) {
        *size = bufferSize;
    }

    return buffer;
}

static bool invoke_init(const char* filename)
{
    int fd = vfs_open(filename, O_RDONLY);
    if (fd < 0) {
        return false;
    }

    struct stat stinfo{};
    if (0 != vfs_stat(fd, &stinfo)) {
        vfs_close(fd);
        return false;
    }

    size_t size = (size_t) stinfo.st_size;
    auto buffer = std::make_unique<uint8_t[]>(size);
    if (-1 == vfs_read(fd, buffer.get(), size)) {
        vfs_close(fd);
        return false;
    }

    vfs_close(fd);

    std::vector<std::string> initArgs{filename};
    auto* proc = ProcessProvider::instance().createProcess("init", buffer.get(), size, std::move(initArgs), PAGESIZE);
    if (proc) {
        Scheduler::instance().waitForPid(proc->pid());
    }

    return true;
}

extern "C"
int kernel_main(void* argument) {

    interface_add("eth0", 0x01010101, 0xffffff00);

    initrd_initialize();

    invoke_init("/bin/init");

    while (1) {
        uint32_t size = 0;
        uint8_t* buffer = recv_file_over_udp(&size);
        if (!buffer) {
            syscall(SYS_NR_SLEEP, 5000);
            continue;
        }

        kprintf("got buffer %p of %d size!\n", buffer, size);
        kprintf("checksum: %08x\n", checksum(buffer, (int) size, 0));

        std::vector<std::string> args{"arg"};
        auto* proc = ProcessProvider::instance().createProcess("main.elf", buffer, size, std::move(args), PAGESIZE);
        free(buffer);

        if (proc) {
            Scheduler::instance().waitForPid(proc->pid());
        }
    }

    return 0;
}

#pragma clang diagnostic pop
