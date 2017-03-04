#ifndef GZOS_TIMER_H
#define GZOS_TIMER_H
#ifdef __cplusplus

#include <lib/kernel/sched/scheduler.h>

class BasicTimer
{
public:
    BasicTimer(int intervalInMs, void* userdata);
    virtual ~BasicTimer(void);

    virtual void start(void);
    virtual void stop(void);

protected:
    virtual bool execute(void* userdata) = 0;

private:
    static bool _timerCallback(void*, BasicTimer* self);

    uint32_t _timerId;
    int _intervalInMs;
    void* _userdata;
    std::atomic_bool _keep_running;
};

template<typename TArgument = void>
class Timer : public BasicTimer
{
public:
    using ActionFunction = bool (*)(TArgument*);
    using ExpireFunction = void (*)(TArgument*);

    Timer(int intervalInMs, ActionFunction action, TArgument* argument = nullptr)
            : BasicTimer(intervalInMs, reinterpret_cast<void*>(argument)),
              _action(action),
              _expiredAction(nullptr),
              _ticks(-1),
              _initialTicks(-1)
    {

    }

    Timer(int intervalInMs, int expirationTicks, ActionFunction action, ExpireFunction expirationAction, TArgument* argument = nullptr)
        : BasicTimer(intervalInMs, argument),
          _action(action),
          _expiredAction(expirationAction),
          _ticks(expirationTicks),
          _initialTicks(expirationTicks)
    {

    }

    virtual ~Timer() = default;

    void stop(void) override {
        BasicTimer::stop();
        _ticks = _initialTicks;
    }

private:
    virtual bool execute(void* userdata) override {
        if (nullptr == _action) {
            return false;
        }

        // in case of no ticks behaviour
        if (-1 == _ticks) {
            return _action(reinterpret_cast<TArgument*>(userdata));
        }

        // in case of ticks enforcement
        _ticks--;
        auto result = _action(reinterpret_cast<TArgument*>(userdata));
        if (!result) {
            return false;
        }

        if (0 == _ticks && _expiredAction) {
            _expiredAction(reinterpret_cast<TArgument*>(userdata));
            return false;
        }

        return true;
    }

    ActionFunction _action;
    ExpireFunction _expiredAction;
    int _ticks;
    int _initialTicks;
};

template<typename TArgument = void>
static inline std::unique_ptr<BasicTimer> createTimer(int ms,
                                                      typename Timer<TArgument>::ActionFunction action,
                                                      TArgument* argument = nullptr)
{
    return std::unique_ptr<BasicTimer>(new Timer<TArgument>(ms, action, argument));
}

template<typename TArgument = void>
static inline std::unique_ptr<BasicTimer> createTimer(int ms, typename Timer<TArgument>::ActionFunction action,
                                                      int ticks, typename Timer<TArgument>::ExpireFunction expiredAction,
                                                      TArgument* argument = nullptr)
{
    return std::unique_ptr<BasicTimer>(new Timer<TArgument>(ms, ticks, action, expiredAction, argument));
}

#endif //cplusplus
#endif //GZOS_TIMER_H
