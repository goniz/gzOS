#ifndef GZOS_TIME_H
#define GZOS_TIME_H
#ifdef __cplusplus
extern "C" {
#endif

#include <sys/time.h>

static inline time_t time_sec_to_ms(time_t sec) {
    return sec * 1000;
}

static inline time_t time_usec_to_ms(useconds_t usec) {
    return usec / 1000;
}

static inline time_t time_timeval_to_ms(struct timeval* timeval) {
    if (!timeval) {
        return 0;
    }

    const time_t sec_ms = time_sec_to_ms(timeval->tv_sec);
    const time_t usec_ms = time_usec_to_ms(timeval->tv_usec);
    return sec_ms + usec_ms;
}

#ifdef __cplusplus
}
#endif //extern "C"
#endif //GZOS_TIME_H
