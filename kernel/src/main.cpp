#include <stdio.h>
#include <lib/kernel/sched/scheduler.h>
#include <ctime>
#include <lib/network/arp.h>
#include <lib/network/interface.h>
#include <lib/network/socket.h>
#include <platform/cpu.h>
#include <lib/network/checksum.h>
#include <platform/malta/mips.h>
#include <platform/malta/malta.h>
#include <lib/kernel/vfs/VirtualFileSystem.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
extern "C" char _end;
extern "C" caddr_t _get_stack_pointer(void);

std::vector<struct ps_ent> getProcessList(int expected_entries)
{
    std::vector<struct ps_ent> buffer((unsigned long)expected_entries);

    int nr_entries = syscall(SYS_NR_PS, buffer.data(), buffer.size() * sizeof(struct ps_ent));
    if (-1 == nr_entries) {
        return {};
    }

    buffer.resize((unsigned long) nr_entries);
    return std::move(buffer);
}

void printProcessList(void)
{
    std::vector<struct ps_ent> proclist = std::move(getProcessList(10));

    InterruptsMutex mutex;
    mutex.lock();
    kprintf("%-7s %-20s %-10s %-10s %s\n",
            "PID", "Name", "State", "CPU Time", "ExitCode");
    for (const auto& proc : proclist)
    {
        kprintf("pid: %d\n", proc.pid);
        kprintf("name: %p\n", proc.name);
        kprintf("state: %p\n", proc.state);
        kprintf("%-7d %-20s %-10s %-10lu %d\n",
                proc.pid, proc.name, proc.state, proc.cpu_time, proc.exit_code);
    }
    mutex.unlock();
}

void printArpCache(void)
{
    InterruptsMutex mutex;
    mutex.lock();
    arp_print_cache();
    mutex.unlock();
}

int ps_main(void* argument)
{
    while (1) {
        printProcessList();
        syscall(SYS_NR_SLEEP, 5000);
    }
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
    if (size) *size = 0;

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

    uint8_t* buffer = (uint8_t *) malloc(bufferSize);
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

    if (size) *size = bufferSize;
    return buffer;
}

extern "C"
int kernel_main(void *argument)
{
    kprintf("Current Stack: %p\n", (void*) _get_stack_pointer());
    kprintf("Start of heap: %p\n", (void*) &_end);

    interface_add("eth0", 0x01010101, 0xffffff00);

//    Scheduler::instance().createKernelThread("top", ps_main, nullptr, 8192);
    vfs_mount("fat32", "/pflash/userdata", "/usr");
    vfs_mount("tmpfs", "none", "/tmp");

    std::vector<const char*> shellArgs{};
    syscall(SYS_NR_EXEC, "/usr/bin/shell", shellArgs.size(), shellArgs.data());

    while (1) {
        uint32_t size = 0;
        uint8_t *buffer = recv_file_over_udp(&size);
        if (!buffer) {
            syscall(SYS_NR_SLEEP, 5000);
            continue;
        }

        kprintf("got buffer %p of %d size!\n", buffer, size);
        kprintf("checksum: %08x\n", ip_compute_csum(buffer, (int) size));

        syscall(SYS_NR_YIELD);

        std::vector<const char*> args{"arg"};
        pid_t pid = syscall(SYS_NR_CREATE_PROCESS, buffer, size, "main.elf", args.size(), args.data());
        kprintf("new pid: %d\n", pid);

        free(buffer);
    }

    return 0;
}

#pragma clang diagnostic pop
