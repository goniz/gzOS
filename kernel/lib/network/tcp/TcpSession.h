#ifndef GZOS_TCPSESSION_H
#define GZOS_TCPSESSION_H

#ifdef __cplusplus

#include <lib/primitives/Event.h>
#include <lib/primitives/Timer.h>
#include <lib/primitives/basic_queue.h>
#include <lib/network/nbuf.h>
#include <lib/network/socket.h>
#include <lib/primitives/EventStream.h>
#include "lib/network/tcp/TcpSequence.h"
#include "lib/network/tcp/tcp_out_segment.h"

enum class TcpState {
    Closed,
    SynSent,
    Established,
    Listening,
    CloseWait
};

const char* tcp_state(TcpState state);

class TcpSession
{
public:
    static std::unique_ptr<TcpSession> createTcpServer(uint16_t localPort);
    static std::unique_ptr<TcpSession> createTcpClient(uint16_t localPort, SocketAddress remoteAddr);
    static std::unique_ptr<TcpSession> createTcpRejector(uint16_t localPort, SocketAddress remoteAddr);
    ~TcpSession();

    bool process_in_segment(NetworkBuffer* nbuf);
    static void handle_out_segment_error(std::shared_ptr<TcpOutSegment> segment, TcpSession* self);

    void close(void);

    TcpState wait_state_changed(void);
    TcpState state(void) const;

    bool queue_out_bytes(NetworkBuffer* nbuf);
    int pop_bytes_in(void* buffer, size_t size);

private:
    TcpSession(uint16_t localPort, SocketAddress remoteAddr, bool releasePortOnClose);

    void set_state(TcpState newState);
    NetworkBuffer* tcp_alloc_nbuf(int flags, size_t data_size);
    bool tcp_transmit(NetworkBuffer* nbuf);
    bool send_syn(void);
    bool send_ack(void);
    bool send_ack_push(const void* buffer, size_t size);
    bool send_rst(NetworkBuffer* nbuf = nullptr);
    void pop_bytes_out(void);
    void reset_inflight_segment();

    TcpState _state;
    uint16_t _localPort;
    SocketAddress _remoteAddr;
    EventStream<TcpState> _stateChanged;
    TcpSequence _seq;
    uint32_t _receive_next_ack;
    uint16_t _receive_window;
    bool _releasePortOnClose;
    basic_queue<NetworkBuffer*> _bytes_in;
    basic_queue<NetworkBuffer*> _bytes_out;
    std::shared_ptr<TcpOutSegment> _segment_inflight;
};


#endif //cplusplus
#endif //GZOS_TCPSESSION_H
