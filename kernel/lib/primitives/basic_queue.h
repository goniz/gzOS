#ifndef GZOS_QUEUE_H
#define GZOS_QUEUE_H

#include <stdint.h>
#include <stddef.h>

// basic_queue C API
typedef void* basic_queue_t;
extern "C" basic_queue_t basic_queue_init(size_t elements);
extern "C" int basic_queue_push(basic_queue_t queue, uintptr_t value, int wait);
extern "C" void basic_queue_push_head(basic_queue_t queue, uintptr_t value);
extern "C" int basic_queue_pop(basic_queue_t queue, uintptr_t* out_value, int wait);
extern "C" size_t basic_queue_capacity(basic_queue_t queue);
extern "C" size_t basic_queue_size(basic_queue_t queue);
extern "C" int basic_queue_empty(basic_queue_t queue);
extern "C" int basic_queue_full(basic_queue_t queue);

// basic_queue<T> C++ API
#ifdef __cplusplus
#include <vector>
#include <platform/kprintf.h>
#include <lib/primitives/Suspendable.h>
#include "lock_guard.h"

template<typename T>
class basic_queue : public Suspendable
{
public:
    using reference = typename std::vector<T>::reference;
    using const_reference = typename std::vector<T>::const_reference;

    basic_queue(size_t capacity)
    {
        _data.reserve(capacity);
    }

    inline bool push(const T& value, bool wait = false) {
        // if wait = true and full, then just wait
        if (this->full() && wait) {
            this->wait();
        }

        // but if the wait is over, we want to double check the we're not full and return false if we do
        // as we wont sleep again..

        if (this->full()) {
            return false;
        }

        {
            lock_guard<InterruptsMutex> guard(_mutex);
            _data.push_back(value);
        }

        this->notifyOne();
        return true;
    }

    inline void push_head(const T& value) {
        {
            lock_guard<InterruptsMutex> guard(_mutex);
            _data.insert(_data.begin(), value);
        }

        this->notifyOne();
    }

    inline bool push(T&& value, bool wait = false) {
        // if wait = true and full, then just wait
        if (this->full() && wait) {
            this->wait();
        }

        // but if the wait is over, we want to double check the we're not full and return false if we do
        // as we wont sleep again..

        if (this->full()) {
            return false;
        }

        {
            lock_guard<InterruptsMutex> guard(_mutex);
            _data.push_back(std::move(value));
        }

        this->notifyOne();
        return true;
    }

    inline bool pop(T& out, bool wait = false) {
        // if wait = true and empty, then just wait
        if (this->empty() && wait) {
            this->wait();
        }

        // but if the wait is over, we want to double check the we're not empty and return false if we do
        // as we wont sleep again..

        if (this->empty()) {
            return false;
        }

        lock_guard<InterruptsMutex> guard(_mutex);
        out = _data.front();
        _data.erase(_data.begin());
        return true;
    }

    inline size_t capacity(void) const  {
        return _data.capacity();
    }

    inline size_t size(void) const {
        return _data.size();
    }

    inline bool empty(void) const {
        return (0 == this->size());
    }

    inline bool full(void) const {
        return (this->capacity() == this->size());
    }

    inline const std::vector<T>& underlying_data(void) const {
        return _data;
    }

private:
    std::vector<T>  _data;
    InterruptsMutex _mutex;
};

#endif // c++
#endif //GZOS_QUEUE_H
