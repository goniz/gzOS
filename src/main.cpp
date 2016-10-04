#include <stdio.h>
#include <lib/kernel/scheduler.h>
#include <ctime>
#include <lib/network/arp.h>
#include <lib/network/interface.h>
#include <lib/network/socket.h>

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
    kprintf("%-7s %-20s %-10s %-10s %-10s %s\n",
            "PID", "Name", "Type", "State", "CPU Time", "ExitCode");
    for (const auto& proc : proclist)
    {
        kprintf("%-7d %-20s %-10s %-10s %-10lu %d\n",
                proc.pid, proc.name, proc.type, proc.state, proc.cpu_time, proc.exit_code);
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

int ps_main(int argc, const char** argv)
{
    while (1) {
        printProcessList();
        syscall(SYS_NR_SLEEP, 5000);
    }
}

int main(int argc, const char** argv)
{
    printf("Current Stack: %p\n", (void*) _get_stack_pointer());
    printf("Start of heap: %p\n", (void*) &_end);

    interface_add("eth0", 0x01010101, 0xffffff00);

    std::vector<const char *> args{};
    syscall(SYS_NR_CREATE_PREEMPTIVE_PROC, "top", ps_main, args.size(), args.data(), 8096);

    int sock = syscall(SYS_NR_SOCKET, AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    printf("sock: %d\n", sock);
    if (-1 == sock) {
        return 0;
    }

    SocketAddress addr{IPADDR_ANY, 1234};
    syscall(SYS_NR_BIND, sock, &addr);

    while (1) {
        SocketAddress clientaddr{};
        uint8_t buf[512];
        memset(buf, 0, sizeof(buf));
        int ret = syscall(SYS_NR_RECVFROM, sock, buf, sizeof(buf), &clientaddr);
        kprintf("read: %d from %08x:%d\n", ret, clientaddr.address, clientaddr.port);
        if (-1 == ret) {
            break;
        }

        if (0 == strncmp((const char *) buf, "bye\n", sizeof(buf))) {
            syscall(SYS_NR_CLOSE, sock);
            break;
        }

        syscall(SYS_NR_SENDTO, sock, buf, ret, &clientaddr);
//        for (int i = 0; i < ret; i++) {
//            kprintf("%02x", buf[i]);
//        }
//        kputs("\n");
    }

    return 0;

    while (1) {
        syscall(SYS_NR_SLEEP, 1000);

        NetworkBuffer* nbuf = ip_alloc_nbuf(0x01010102, 64, IPPROTO_ICMP, 100);
        memset(nbuf->l4_offset, 0xAA, nbuf_size_from(nbuf, nbuf->l4_offset));
        ip_output(nbuf);
        nbuf_free(nbuf);
    }

    while (1) {
        syscall(SYS_NR_SLEEP, 2000);

        printProcessList();
        printArpCache();
    }
}

#pragma clang diagnostic pop
