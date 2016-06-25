#include <stdio.h>
#include <lib/scheduler/scheduler.h>
#include <platform/clock.h>
#include <lib/primitives/interrupts_guard.h>

extern "C" char _end;
extern "C" caddr_t _get_stack_pointer(void);

static ProcessScheduler scheduler(10, 10);

static int dummy1ProcMain(int argc, const char** argv)
{
    while (true) {
//        InterruptGuard guard;
        // kprintf("Dummy1Proc: count %ld\n", clock_get_ms());
//		kputs("DummyProc1\n");
    }
}

__attribute__((used))
static int dummy2ProcMain(int argc, const char** argv)
{
    while (true) {
        // InterruptGuard guard;
        // kprintf("Dummy2Proc: count %ld\n", clock_get_ms());
		kputs("DummyProc2\n");
    }
}


int main(int argc, const char** argv)
{
    printf("Current Stack: %p\n", (void*) _get_stack_pointer());
    printf("Start of heap: %p\n", (void*) &_end);

    scheduler.setDebugMode();

    // pid_t dummy1Pid = 
	scheduler.createPreemptiveProcess("Dummy1Proc", dummy1ProcMain, {}, 1024);
    // printf("dummy1 pid: %d\n", dummy1Pid);

    // pid_t dummy2Pid =
//	scheduler.createPreemptiveProcess("Dummy2Proc", dummy2ProcMain, {}, 1024);
    // printf("dummy2 pid: %d\n", dummy2Pid);

    clock_set_handler(ProcessScheduler::onTickTimer, &scheduler);
    return 0;
}

