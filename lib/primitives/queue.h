//
// Created by gz on 6/12/16.
//

#ifndef GZOS_QUEUE_H
#define GZOS_QUEUE_H

#include <vector>
#include <cstddef>
#include <platform/kprintf.h>

template<typename T>
class queue
{
public:
    using reference = typename std::vector<T>::reference;
    using const_reference = typename std::vector<T>::const_reference;

    queue(size_t capacity)
    {
        _data.reserve(capacity);
    }

    inline bool push(const T& value) {
        if (this->full()) {
            return false;
        }

        _data.push_back(value);
        return true;
    }

//    inline bool push(T&& value) {
//        if (this->full()) {
//            return false;
//        }
//
//        _data.push_back(std::move(value));
//        return true;
//    }

    inline bool pop(T& out) {
        if (this->empty()) {
            return false;
        }

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
    std::vector<T> _data;
};


#endif //GZOS_QUEUE_H
