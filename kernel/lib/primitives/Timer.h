#ifndef GZOS_TIMER_H
#define GZOS_TIMER_H
#ifdef __cplusplus

#include <lib/kernel/sched/scheduler.h>

class BasicTimer
{
public:
    BasicTimer(int intervalInMs, void* userdata);
    virtual ~BasicTimer() = default;

    void start();
    void stop();

protected:
    virtual bool execute(void* userdata) = 0;

private:
    static bool _timerCallback(void*, BasicTimer* self);

    int _intervalInMs;
    void* _userdata;
    bool _keep_running = false;
};

template<typename T>
class Timer : public BasicTimer
{
public:
    Timer(int intervalInMs, T&& action)
            : BasicTimer(intervalInMs, nullptr),
              _action(std::move(action))
    {

    }

    virtual ~Timer() = default;

private:
    virtual bool execute(void* userdata) override {
        return _action();
    }

    T _action;
};

#endif //cplusplus
#endif //GZOS_TIMER_H
