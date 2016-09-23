#include <stdio.h>
#include <lib/kernel/scheduler.h>
#include <lib/syscall/syscall.h>
#include <ctime>
#include <lib/kernel/VirtualFileSystem.h>
#include <fcntl.h>
#include <lib/network/arp.h>
#include <lib/network/interface.h>

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

int main(int argc, const char** argv)
{
    printf("Current Stack: %p\n", (void*) _get_stack_pointer());
    printf("Start of heap: %p\n", (void*) &_end);

    int fd = -1;

    printf("new fd is %d\n", fd = vfs_open("/dev/null", O_RDONLY));
    vfs_close(fd);

    printf("new fd is %d\n", fd = vfs_open("/dev/null", O_RDONLY));
    vfs_close(fd);

    printf("new fd is %d\n", fd = vfs_open("/dev/null", O_RDONLY));
    vfs_close(fd);

    interface_add("eth0", 0x01010101, 0xffffff00);
    return 0;

    while (1) {
        scheduler_sleep(2000);

        printProcessList();
        printArpCache();
    }
}

#pragma clang diagnostic pop
