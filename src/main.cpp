#include <stdio.h>
#include <lib/scheduler/scheduler.h>
#include <platform/clock.h>

extern "C" char _end;
extern "C" caddr_t _get_stack_pointer(void);

static ProcessScheduler scheduler(10, 10);

int main(int argc, const char** argv)
{
    printf("Current Stack: %p\n", (void *) _get_stack_pointer());
    printf("Start of heap: %p\n", &_end);

    clock_set_handler(ProcessScheduler::onTickTimer, &scheduler);
    return 0;
}

