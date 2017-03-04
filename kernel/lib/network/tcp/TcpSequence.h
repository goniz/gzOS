#ifndef GZOS_TCPSEQUENCE_H
#define GZOS_TCPSEQUENCE_H

#include <stdint.h>
#include <atomic>

#ifdef __cplusplus

class TcpSequence
{
public:
    TcpSequence(void);
    ~TcpSequence(void) = default;

    uint32_t seq(void) const;
    uint32_t prev(void) const;

    TcpSequence& advance(unsigned long bytes);

private:
    uint32_t _sequence;
    uint32_t _prev;
};

#endif //cplusplus
#endif //GZOS_TCPSEQUENCE_H
