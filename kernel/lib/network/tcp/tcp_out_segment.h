#ifndef GZOS_TCP_OUT_SEGMENT_H
#define GZOS_TCP_OUT_SEGMENT_H

#ifdef __cplusplus

#include <memory>
#include <atomic>
#include <lib/network/nbuf.h>
#include <lib/primitives/Timer.h>

struct TcpOutSegment;
using TcpOutSegmentErrorFunc = void (*)(std::shared_ptr<TcpOutSegment> segment, void* argument);

struct TcpOutSegment {
    std::atomic_bool valid;
    NetworkBuffer* packet;
    int transmissions;
    int threshold;
    int intervalInMs;
    TcpOutSegmentErrorFunc callback;
    void* callback_arg;
    std::unique_ptr<BasicTimer> retransmissionTimer;
};

std::shared_ptr<TcpOutSegment> tcp_out_queue_packet(NetworkBuffer* nbuf, TcpOutSegmentErrorFunc errorCallback, void* arg);

#endif //cplusplus
#endif //GZOS_TCP_OUT_SEGMENT_H
