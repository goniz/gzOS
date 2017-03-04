#include "Timer.h"

BasicTimer::BasicTimer(int intervalInMs, void* userdata)
        : _timerId(0),
          _intervalInMs(intervalInMs),
          _userdata(userdata),
          _keep_running(false)
{

}

BasicTimer::~BasicTimer(void) {
    this->stop();
}

void BasicTimer::start(void) {
    if (0 != _timerId) {
        this->stop();
    }

    _keep_running = true;
    _timerId = scheduler_set_timeout(_intervalInMs, (timeout_callback_t)BasicTimer::_timerCallback, this);
}

void BasicTimer::stop(void) {
    _keep_running = false;
    if (0 != _timerId) {
        scheduler_unset_timeout(_timerId);
        _timerId = 0;
    }
}

bool BasicTimer::_timerCallback(void*, BasicTimer* self) {
    if (!self->_keep_running) {
        return false;
    }

    return (self->_keep_running = self->execute(self->_userdata));
}
