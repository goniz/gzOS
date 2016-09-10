#include <stdio.h>
#include <lib/scheduler/scheduler.h>
#include <platform/clock.h>
#include <lib/syscall/syscall.h>
#include <lib/scheduler/signals.h>
#include <platform/cpu.h>
#include <platform/malta/mips.h>
#include <platform/pci/pci.h>
#include <unistd.h>
#include <lib/network/packet_pool.h>
#include <lib/network/ethernet_layer.h>
#include <ctime>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
extern "C" char _end;
extern "C" caddr_t _get_stack_pointer(void);
extern "C" int kill(int pid, int sig);

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
    printf("%-7s %-20s %-10s %-10s %-10s %s\n",
           "PID", "Name", "Type", "State", "CPU Time", "ExitCode");
    for (const auto& proc : proclist)
    {
        printf("%-7d %-20s %-10s %-10s %-10lu %d\n",
               proc.pid, proc.name, proc.type, proc.state, proc.cpu_time, proc.exit_code);
    }
    mutex.unlock();
}

static basic_queue<IncomingPacketBuffer> gInputPackets(10);
static std::atomic<uint32_t> gCounter(0);

__attribute__((used))
static int EthernetEchoServer(int argc, const char **argv)
{
    while (true) {
        IncomingPacketBuffer incomingPacketBuffer;
        if (!gInputPackets.pop(incomingPacketBuffer, true)) {
            continue;
        }

        gCounter++;
        ethernet_send_packet("eth0", incomingPacketBuffer.header->src, incomingPacketBuffer.header->type, &incomingPacketBuffer.buffer);
    }
}

static timeout_callback_ret statsPrinterTimerCb(void *arg) {
    uint32_t value = gCounter;
    uint64_t now = clock_get_ms();
    kprintf("%lld: stats: rx packets %d\n", now, value);
    return TIMER_KEEP_GOING;
}

void arp_handler(void* user_ctx, IncomingPacketBuffer* incomingPacket)
{
    gInputPackets.push(*incomingPacket);
}

int main(int argc, const char** argv)
{
    printf("Current Stack: %p\n", (void*) _get_stack_pointer());
    printf("Start of heap: %p\n", (void*) &_end);

    ethernet_register_handler(0x0806, arp_handler, NULL);

    std::vector<const char*> args{};
    syscall(SYS_NR_CREATE_RESPONSIVE_PROC, "EthernetEchoServer", EthernetEchoServer, args.size(), args.data(), 8096);

    scheduler_set_timeout(1000, statsPrinterTimerCb, nullptr);

    while (1) {
        scheduler_sleep(1000);

        printProcessList();

        InterruptsMutex mutex;
        mutex.lock();
        time_t time1;
        time(&time1);
        struct tm cpuTime{};
        mktime(&cpuTime);
        printf("time1: %s\n", asctime(localtime(&time1)));
        mutex.unlock();
    }

//    printProcessList();
//
//    kill(getpid(), SIG_STOP);
//    syscall(SYS_NR_YIELD);
    return 0;
}


#pragma clang diagnostic pop
