#include "platform/clock.h"
#include <stdint.h>
#include "platform/kprintf.h"

static uint32_t _start = 0;
static uint32_t _prev = 0;

void clock_stopwatch_start() {
	_start = _prev = clock_get_raw_count();
	kprintf("clock: started at %lu\n", _start);
}

void clock_stopwatch_tag(const char* msg) {
	uint32_t now = clock_get_raw_count();

	kprintf("clock: %s - elapsed %lu\n", msg, now - _prev);
    _prev = now;
//	kprintf("clock: now %lu\n", now);
}

void clock_stopwatch_stop() {
	uint32_t now = clock_get_raw_count();

	kprintf("clock: overall elapsed %lu\n", now - _start);
	_start = 0;
    _prev = 0;
}
