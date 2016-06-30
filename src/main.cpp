#include <stdio.h>
#include <lib/scheduler/scheduler.h>
#include <platform/clock.h>
#include <lib/primitives/interrupts_mutex.h>
#include <lib/syscall/syscall.h>
#include <unistd.h>
#include <sys/signal.h>
#include <lib/scheduler/signals.h>

extern "C" char _end;
extern "C" caddr_t _get_stack_pointer(void);
extern "C" int kill(int pid, int sig);

__attribute__((used))
static int dummy2ProcMain(int argc, const char** argv)
{
    int test = 0;
    while (true) {
        if (!test) {
            kputs("DummyProc2\n");
//            test = 1;
        }
    }
}

__attribute__((used))
static int dummy1ProcMain(int argc, const char** argv)
{
    while (true) {
		kprintf("DummyProc1 pid %d\n", getpid());
    }
}

__attribute__((used))
static int dummyResponsiveProcMain(int argc, const char** argv)
{
    while (true) {
        kputs("Responsive Proc\n");
        int retval = syscall(SYS_NR_YIELD);
        kprintf("retval: %d\n", retval);
    }
}

DEFINE_SYSCALL(10, test)
{
    SYSCALL_ARG(const char*, str);

    kprintf("%s: arg %s\n", __FUNCTION__, str);
    return 0;
}

int main(int argc, const char** argv)
{
    printf("Current Stack: %p\n", (void*) _get_stack_pointer());
    printf("Start of heap: %p\n", (void*) &_end);

//    clock_delay_ms(10);

    std::vector<const char*> args{};
    pid_t dummy1_pid = syscall(SYS_NR_CREATE_PREEMPTIVE_PROC, "Dummy1Proc", dummy1ProcMain, args.size(), args.data(), 4096);
    pid_t dummy2_pid = syscall(SYS_NR_CREATE_PREEMPTIVE_PROC, "Dummy2Proc", dummy2ProcMain, args.size(), args.data(), 4096);
    pid_t dummy3_pid = syscall(SYS_NR_CREATE_RESPONSIVE_PROC, "Dummy3Proc", dummyResponsiveProcMain, args.size(), args.data(), 4096);

    clock_delay_ms(5000);

    kill(dummy1_pid, SIG_KILL);
    kill(dummy2_pid, SIG_KILL);
    kill(dummy3_pid, SIG_KILL);

    while (1);
}

