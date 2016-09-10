#ifndef GZOS_EVENT_H
#define GZOS_EVENT_H

#ifdef __cplusplus

class Event {
    void wait(int timeout_ms);
    void raise(void);
};

#endif //cplusplus

#endif //GZOS_EVENT_H
