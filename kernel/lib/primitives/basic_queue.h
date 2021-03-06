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
#include <algorithm>
#include "LockGuard.h"
#include "SuspendableMutex.h"

template<typename T>
class basic_queue : public Suspendable
{
public:
    using reference = typename std::vector<T>::reference;
    using const_reference = typename std::vector<T>::const_reference;

    basic_queue() = default;
    basic_queue(size_t capacity)
    {
        _data.reserve(capacity);
    }

    bool push(const T& value, bool wait = false) {
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
            LockGuard guard(_mutex);
            _data.push_back(value);
        }

        this->notifyOne(0);
        return true;
    }

    void push_head(const T& value) {
        {
            LockGuard guard(_mutex);
            _data.insert(_data.begin(), value);
        }

        this->notifyOne(0);
    }

    bool push(T&& value, bool wait = false) {
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
            LockGuard guard(_mutex);
            _data.push_back(std::move(value));
        }

        this->notifyOne(0);
        return true;
    }

    bool pop(T& out, bool wait = false) {
        // if wait = true and empty, then just wait
        if (this->empty() && wait) {
            this->wait();
        }

        // but if the wait is over, we want to double check the we're not empty and return false if we do
        // as we wont sleep again..

        if (this->empty()) {
            return false;
        }

        LockGuard guard(_mutex);
        out = std::move(_data.front());
        _data.erase(_data.begin());
        return true;
    }

    bool peek(T& out, bool wait = false) {
        if (this->empty() && wait) {
            this->wait();
        }

        if (this->empty()) {
            return false;
        }

        LockGuard guard(_mutex);
        out = _data.front();
        return true;
    }

    bool peek_back(T& out, bool wait = false) {
        if (this->empty() && wait) {
            this->wait();
        }

        if (this->empty()) {
            return false;
        }

        LockGuard guard(_mutex);
        out = _data.back();
        return true;
    }

    template<typename TFunc>
    void remove(TFunc comparatorFunc) {
        LockGuard guard(_mutex);

        auto results = std::remove_if(_data.begin(), _data.end(), comparatorFunc);
        _data.erase(results, _data.end());
    }

    void reserve(size_t size) {
        LockGuard guard(_mutex);
        _data.reserve(size);
    }

    inline size_t capacity(void) const  {
        return _data.capacity();
    }

    inline size_t size(void) const {
        return _data.size();
    }

    inline bool empty(void) const {
        return _data.empty();
    }

    inline bool full(void) const {
        return (this->capacity() == this->size());
    }

    inline const std::vector<T>& underlying_data(void) const {
        return _data;
    }

private:
    std::vector<T>  _data;
    SuspendableMutex _mutex;
};

#endif // c++
#endif //GZOS_QUEUE_H
