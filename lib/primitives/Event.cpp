#include "Event.h"

void Event::wait(void) {
    Suspendable::wait();
}

void Event::raise(void) {
    this->notifyAll();
}
