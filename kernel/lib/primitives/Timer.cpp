#include "Timer.h"

BasicTimer::BasicTimer(int intervalInMs, void* userdata)
        : _intervalInMs(intervalInMs),
          _userdata(userdata),
          _keep_running(false)
{

}

void BasicTimer::start() {
    _keep_running = true;
    scheduler_set_timeout(_intervalInMs, (timeout_callback_t)BasicTimer::_timerCallback, this);
}

void BasicTimer::stop() {
    _keep_running = false;
}

bool BasicTimer::_timerCallback(void*, BasicTimer* self) {
    if (!self->_keep_running) {
        return false;
    }

    return (self->_keep_running = self->execute(self->_userdata));
}
