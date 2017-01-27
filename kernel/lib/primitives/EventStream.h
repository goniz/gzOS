#ifndef GZOS_EVENTSTREAM_H
#define GZOS_EVENTSTREAM_H

#ifdef __cplusplus

#include "Suspendable.h"

template<typename T>
class EventStream : private Suspendable
{
    static_assert(sizeof(uintptr_t) == sizeof(T), "T of EventStream<T> must be compatible with uintptr_t");

public:
    EventStream(void) = default;
    virtual ~EventStream(void) = default;

    T get(void) {
        return static_cast<T>(this->wait());
    }

    void emit(T value) {
        this->notifyAll(static_cast<uintptr_t>(value));
    }
};

#endif //cplusplus
#endif //GZOS_EVENTSTREAM_H
