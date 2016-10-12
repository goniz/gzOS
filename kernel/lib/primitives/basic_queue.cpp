#include "basic_queue.h"

static inline basic_queue<uintptr_t>* as_queue(basic_queue_t queue) {
    return (basic_queue<uintptr_t>*)queue;
}

extern "C"
basic_queue_t basic_queue_init(size_t elements)
{
    return new basic_queue<uintptr_t>(elements);
}

extern "C"
int basic_queue_push(basic_queue_t queue, uintptr_t value, int wait) {
    return as_queue(queue)->push(value, wait != 0);
}

extern "C"
void basic_queue_push_head(basic_queue_t queue, uintptr_t value) {
    as_queue(queue)->push_head(value);
}

extern "C"
int basic_queue_pop(basic_queue_t queue, uintptr_t *out_value, int wait) {
    return as_queue(queue)->pop(*out_value, wait != 0);
}

extern "C"
size_t basic_queue_capacity(basic_queue_t queue) {
    return as_queue(queue)->capacity();
}

extern "C"
size_t basic_queue_size(basic_queue_t queue) {
    return as_queue(queue)->size();
}

extern "C"
int basic_queue_empty(basic_queue_t queue) {
    return as_queue(queue)->empty() ? 1 : 0;
}

extern "C"
int basic_queue_full(basic_queue_t queue) {
    return as_queue(queue)->full() ? 1 : 0;
}
