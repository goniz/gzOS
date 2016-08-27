#include <stdio.h>
#include <lib/scheduler/scheduler.h>
#include <platform/clock.h>
#include <lib/syscall/syscall.h>
#include <lib/scheduler/signals.h>
#include <platform/cpu.h>
#include <platform/malta/mips.h>
#include <platform/pci/pci.h>
#include <unistd.h>

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

    printf("%-7s %-20s %-10s %-10s %s\n",
           "PID", "Name", "Type", "State", "ExitCode");
    for (const auto& proc : proclist)
    {
        printf("%-7d %-20s %-10s %-10s %d\n",
               proc.pid, proc.name, proc.type, proc.state, proc.exit_code);
    }
}

basic_queue<int> myqueue(10);

_GLIBCXX_NORETURN
static int ProducerMain(int argc, const char **argv)
{
    int i = 0;
    while (true) {
        clock_delay_ms(1000);
//        printProcessList();
        printf("pushing an %d\n", i);
        myqueue.push(i++);
    }
}
__attribute__((used))
static int ResponsiveConsumer(int argc, const char **argv)
{
    while (true) {
        int number = 0;
        myqueue.pop(number, true);
        printf("poped an %d\n", number);
//        printProcessList();
    }
}

#define MY_STRING(x)   ({                                               \
                            __attribute__((section(".strings"),used))   \
                            static const char* s = (x);                 \
                            s;                                          \
                        })


int main(int argc, const char** argv)
{
    printf("Current Stack: %p\n", (void*) _get_stack_pointer());
    printf("Start of heap: %p\n", (void*) &_end);

    const char* mystring = MY_STRING("Test");
    printf("str: %s\n", mystring);

    std::vector<const char*> args{};
    syscall(SYS_NR_CREATE_PREEMPTIVE_PROC, "Producer", ProducerMain, args.size(), args.data(), 4096);
    syscall(SYS_NR_CREATE_RESPONSIVE_PROC, "Consumer", ResponsiveConsumer, args.size(), args.data(), 4096);

    syscall(SYS_NR_YIELD);
    clock_delay_ms(5000);
    printProcessList();

    kill(getpid(), SIG_STOP);
    syscall(SYS_NR_YIELD);
    return 0;
}


#pragma clang diagnostic pop