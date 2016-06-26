#include <stdio.h>
#include <lib/scheduler/scheduler.h>
#include <platform/clock.h>
#include <lib/primitives/interrupts_mutex.h>
#include <lib/syscall/syscall.h>

extern "C" char _end;
extern "C" caddr_t _get_stack_pointer(void);

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
		kputs("DummyProc1\n");
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
    printf("new pid: %d\n", syscall(SYS_NR_CREATE_PREEMPTIVE_PROC, "Dummy1Proc", dummy1ProcMain, args.size(), args.data(), 4096));
    printf("new pid: %d\n", syscall(SYS_NR_CREATE_PREEMPTIVE_PROC, "Dummy2Proc", dummy2ProcMain, args.size(), args.data(), 4096));
    printf("new pid: %d\n", syscall(SYS_NR_CREATE_RESPONSIVE_PROC, "Dummy3Proc", dummyResponsiveProcMain, args.size(), args.data(), 4096));

    //	scheduler.createPreemptiveProcess("Dummy1Proc", dummy1ProcMain, {}, 4096);
//	scheduler.createPreemptiveProcess("Dummy3Proc", dummy2ProcMain, {}, 4096);
//  scheduler.createResponsiveProcess("ResponsiveProc", dummyResponsiveProcMain, {}, 4096);

    while (1);
}

