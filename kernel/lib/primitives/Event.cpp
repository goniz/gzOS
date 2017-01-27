#include "Event.h"

Event::Event(void)
    : _flag(false)
{

}

void Event::wait(void) {
    while (!_flag) {
        if (0 != Suspendable::wait()) {
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
