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
        : _sequence(0),
          _prev(0)
{

}

TcpSequence& TcpSequence::operator=(uint32_t seq) {
    _sequence = seq;
    _prev = 0;
    return *this;
}
