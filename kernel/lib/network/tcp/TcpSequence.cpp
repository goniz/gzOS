#include "TcpSequence.h"

uint32_t TcpSequence::seq(void) const {
    return _sequence;
}

uint32_t TcpSequence::prev(void) const {
    return _prev;
}

TcpSequence& TcpSequence::advance(unsigned long bytes) {
    _prev = _sequence;
    _sequence += bytes;
    return *this;
}

TcpSequence::TcpSequence()
        : _sequence(0)
{

}
