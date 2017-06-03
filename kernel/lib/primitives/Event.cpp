#include "Event.h"

Event::Event(void)
    : _flag(false)
{

}

void Event::wait(void) {
    while (!_flag) {
        const int result = Suspendable::wait();
        if (0 != result) {
            break;
        }
    }
}

void Event::raise(void) {
    _flag = true;
    this->notifyAll(1);
}

void Event::reset(void) {
    _flag = false;
    this->notifyAll(0);
}

bool Event::is_set(void) const {
    return _flag;
}
