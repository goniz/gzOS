#include <stdio.h>
#include <lib/scheduler/scheduler.h>
#include <platform/clock.h>
#include <lib/syscall/syscall.h>
#include <lib/scheduler/signals.h>
#include <platform/cpu.h>
#include <platform/malta/mips.h>
#include <platform/pci/pci.h>

extern "C" char _end;
extern "C" caddr_t _get_stack_pointer(void);
extern "C" int kill(int pid, int sig);

__attribute__((used))
static int dummy2ProcMain(int argc, const char** argv)
{
    int test = 0;
    while (true) {
        if (!test) {
//            kputs("DummyProc2\n");
//            test = 1;
        }
    }
}

__attribute__((used))
static int dummy1ProcMain(int argc, const char** argv)
{
    while (true) {
//		kprintf("DummyProc1 pid %d\n", getpid());
    }
}

__attribute__((used))
static int dummyResponsiveProcMain(int argc, const char** argv)
{
    return 1337;
    while (true) {
//        kputs("Responsive Proc\n");
        /*int retval = */ syscall(SYS_NR_YIELD);
//        kprintf("retval: %d\n", retval);
    }
}

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

#define MY_STRING(x)   ({                                               \
                            __attribute__((section(".strings"),used))   \
                            static const char* s = (x);                 \
                            s;                                          \
                        })


static void print_sr()
{
    unsigned sr = mips32_get_c0(C0_STATUS);
    kprintf ("Status   : CU0:%d BEV:%d NMI:%d IM:$%02x KSU:%d ERL:%d EXL:%d IE:%d\n",
             (sr & SR_CU0) >> SR_CU0_SHIFT,
             (sr & SR_BEV) >> SR_BEV_SHIFT,
             (sr & SR_NMI) >> SR_NMI_SHIFT,
             (sr & SR_IMASK) >> SR_IMASK_SHIFT,
             (sr & SR_KSU_MASK) >> SR_KSU_SHIFT,
             (sr & SR_ERL) >> SR_ERL_SHIFT,
             (sr & SR_EXL) >> SR_EXL_SHIFT,
             (sr & SR_IE) >> SR_IE_SHIFT);
}

static void print_pci_irqs()
{
    const auto& devices = platform_pci_bus()->devices();
    for (const auto& dev : devices) {
        printf("%04x:%04x pin %d line %d\n", dev.vendorId(), dev.deviceId(), dev.readByte(PCI_INTERRUPT_PIN), dev.readByte(PCI_INTERRUPT_LINE));
    }
}


int main(int argc, const char** argv)
{
    printf("Current Stack: %p\n", (void*) _get_stack_pointer());
    printf("Start of heap: %p\n", (void*) &_end);

//    clock_delay_ms(10);

    const char* mystring = MY_STRING("Test");
    printf("str: %s\n", mystring);

    std::vector<const char*> args{};
    pid_t dummy1_pid = syscall(SYS_NR_CREATE_PREEMPTIVE_PROC, "Dummy1Proc", dummy1ProcMain, args.size(), args.data(), 4096);
    pid_t dummy2_pid = syscall(SYS_NR_CREATE_PREEMPTIVE_PROC, "Dummy2Proc", dummy2ProcMain, args.size(), args.data(), 4096);
    pid_t dummy3_pid = syscall(SYS_NR_CREATE_RESPONSIVE_PROC, "Dummy3Proc", dummyResponsiveProcMain, args.size(), args.data(), 4096);

    clock_delay_ms(1000);
    printProcessList();
    clock_delay_ms(3000);

    kill(dummy1_pid, SIG_KILL);
    kill(dummy2_pid, SIG_KILL);
    kill(dummy3_pid, SIG_KILL);

    syscall(SYS_NR_YIELD);
    clock_delay_ms(1000);
    printProcessList();

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
    while (1) {}
#pragma clang diagnostic pop
}

