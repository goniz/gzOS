#include <platform/clock.h>
#include <algorithm>
#include <platform/panic.h>
#include "SystemTimer.h"
#include "Scheduler.h"
#include "proc.h"

SystemTimer& SystemTimer::instance(void) {
    auto* self = system_timer_get();

    if (NULL == self) {
        panic("SystemTimer instance is NULL. yayks..");
    }

    return *self;
}

SystemTimer::SystemTimer(ProcessProvider& processProvider, Scheduler& scheduler)
    : _timerProc(processProvider.createKernelProcess("timer", {})),
      _timers(),
      _idAllocator(1000, 2000)
{
    _timers.reserve(SCHED_INITIAL_TIMERS_SIZE);

    scheduler.addTickHandler(SystemTimer::onTimerTick, this);
}

uint32_t SystemTimer::addTimer(int timeout_ms, TimeoutCallbackFunc cb, void* arg) {

    uint64_t now = clock_get_ms();
    unsigned int id = _idAllocator.allocate();

    {
        InterruptsMutex mutex(true);
        _timers.push_back(TimerControlBlock{id, (uint64_t) timeout_ms, now + timeout_ms, cb, arg});
    }

    return id;
}

void SystemTimer::removeTimer(uint32_t timerId) {
    InterruptsMutex mutex(true);
    {
        auto remove_iter = std::remove_if(_timers.begin(), _timers.end(), [timerId](const auto& item) -> bool {
            return item.id == timerId;
        });

        _timers.erase(remove_iter, _timers.end());
    }
}

bool SystemTimer::onTimerTick(void* argument, struct user_regs** regs) {
    SystemTimer* self = (SystemTimer*)argument;

    if (!self) {
        return false;
    }

    self->executeTimers();

    return false;
}

void SystemTimer::executeTimers(void) {

    Thread* current_thread = Scheduler::instance().CurrentThread();

    auto iter = _timers.begin();
    while (iter != _timers.end()) {
        bool remove = false;
        uint64_t now = clock_get_ms();
        if (now >= iter->target_ms) {
            if (iter->callbackFunc) {
                _timerProc->activateProcess();
                remove = !iter->callbackFunc(iter->arg);

                if (!remove) {
                    iter->target_ms = now + iter->timeout_ms;
                }
            }
        }

        if (remove) {
            iter = _timers.erase(iter);
        } else {
            ++iter;
        }
    }

    if (current_thread) {
        current_thread->proc().activateProcess();
    }
}

Process& SystemTimer::TimerProcess(void) {
    assert(NULL != _timerProc);
    return *_timerProc;
}
