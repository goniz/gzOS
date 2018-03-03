#ifndef GZOS_TCPSESSION_H
#define GZOS_TCPSESSION_H

#ifdef __cplusplus

#include <lib/primitives/Event.h>
#include <lib/primitives/Timer.h>
#include <lib/primitives/basic_queue.h>
#include <lib/network/nbuf.h>
#include <lib/network/socket.h>
#include <lib/primitives/EventStream.h>
#include <tcp/states/TcpState.h>
#include "lib/network/tcp/TcpSequence.h"

class TcpSession
{
public:
    static std::unique_ptr<TcpSession> createTcpServer(uint16_t localPort, int backlog);
    static std::unique_ptr<TcpSession> createTcpClient(uint16_t localPort, SocketAddress remoteAddr);
    static std::unique_ptr<TcpSession> createTcpRejector(uint16_t localPort, SocketAddress remoteAddr);
    static std::unique_ptr<TcpSession> createTcpClientAcceptor(uint16_t localPort, SocketAddress remoteAddr, uint32_t seq);
    ~TcpSession();

    bool process_in_segment(NetworkBuffer* nbuf);
    int poll(bool* read_ready, bool* write_ready);

    bool send_syn(void);
    bool send_ack(void);
    bool send_syn_ack(void);
    bool send_ack_push(const void* buffer, size_t size);
    bool send_rst(NetworkBuffer* nbuf = nullptr);
    bool send_fin(void);

    void close(void);

    template<typename TState, typename... TArgs>
    void set_state(TArgs... args) {
        this->set_state(std::make_unique<TState>(*this, std::forward<TArgs>(args)...));
    };

    void set_state(std::unique_ptr<TcpState> newState);
    TcpStateEnum wait_state_changed(void);
    TcpStateEnum state(void) const;

    bool is_seq_match(const tcp_t* tcp) const;
    TcpSequence& seq(void);
    uint32_t& receive_next_ack(void);
    uint16_t& receive_window(void);
    uint16_t localPort(void) const;
    SocketAddress remoteAddr(void) const;

    int acceptNewClient(bool wait);

    int push_input_bytes(const uint8_t* buffer, size_t size);
    int push_output_bytes(const uint8_t* buffer, size_t size);
    int pop_output_bytes(uint8_t* buffer, size_t size, bool wait = false);
    int pop_input_bytes(uint8_t* buffer, size_t size, bool wait = false);

private:
    TcpSession(uint16_t localPort,
               SocketAddress remoteAddr,
               bool releasePortOnClose);
    TcpSession(uint16_t localPort);

    NetworkBuffer* tcp_alloc_nbuf(int flags, size_t data_size);
    bool tcp_transmit(NetworkBuffer* nbuf);

    struct BufferDesc {
        std::vector<uint8_t> buffer;
        EventStream<int> eventStream;
    };

    std::unique_ptr<TcpState> _state;
    uint16_t _localPort;
    SocketAddress _remoteAddr;
    EventStream<TcpStateEnum> _stateChanged;
    TcpSequence _seq;
    uint32_t _receive_next_ack;
    uint16_t _receive_window;
    bool _releasePortOnClose;

    BufferDesc _inputBuffer;
    BufferDesc _outputBuffer;
};


#endif //cplusplus
#endif //GZOS_TCPSESSION_H
