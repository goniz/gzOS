#include <algorithm>
#include <sys/param.h>
#include <checksum.h>
#include <platform/drivers.h>
#include "lib/network/tcp/tcp.h"
#include <lib/network/tcp/states/TcpStateEnum.h>
#include "lib/network/tcp/TcpSession.h"
#include <lib/network/tcp/states/TcpStateSynSent.h>
#include <lib/network/tcp/states/TcpStateClosed.h>
#include <lib/network/tcp/states/TcpStateListening.h>
#include <tcp/states/TcpStateSynReceived.h>
#include "lib/network/tcp/tcp_sessions.h"

/*
 * NOTES:
 * 1) generate packet
 * 2) put on queue with retransmission max count + ack event flag
 * 3) output task (?) de-queues the nbuf to send
 * 4) tick every X and check if a retransmission is needed
 * 5) if retransmission count reached max, call some callback
 * 6) if a segment is ACK'ed, signal on some event flag that will remove the segment from the send queue
 *
 * current status: completely broken.. ;)
 */

TcpSession::TcpSession(uint16_t localPort,
                       SocketAddress remoteAddr,
                       bool releasePortOnClose)
    : _state(nullptr),
      _localPort(localPort),
      _remoteAddr(remoteAddr),
      _seq(),
      _receive_next_ack(0),
      _receive_window(29200),
      _releasePortOnClose(releasePortOnClose)
{
    _inputBuffer.buffer.reserve(_receive_window);
    _outputBuffer.buffer.reserve(_receive_window);

    _state = std::make_unique<TcpStateClosed>(*this);
}

TcpSession::TcpSession(uint16_t localPort)
    : TcpSession(localPort, {0, 0}, true)
{

}

std::unique_ptr<TcpSession> TcpSession::createTcpClient(uint16_t localPort, SocketAddress remoteAddr) {
    auto session = std::unique_ptr<TcpSession>(new (std::nothrow) TcpSession(localPort, remoteAddr, true));
    if (!session) {
        return {};
    }

    session->set_state<TcpStateSynSent>();

    return std::move(session);
}

std::unique_ptr<TcpSession> TcpSession::createTcpRejector(uint16_t localPort, SocketAddress remoteAddr) {
    return std::unique_ptr<TcpSession>(new (std::nothrow) TcpSession(localPort, remoteAddr, false));
}

std::unique_ptr<TcpSession> TcpSession::createTcpServer(uint16_t localPort, int backlog) {
    auto session = std::unique_ptr<TcpSession>(new (std::nothrow) TcpSession(localPort));
    if (!session) {
        return {};
    }

    session->set_state<TcpStateListening>(backlog);

    return std::move(session);
}

std::unique_ptr<TcpSession> TcpSession::createTcpClientAcceptor(uint16_t localPort,
                                                                SocketAddress remoteAddr,
                                                                uint32_t seq)
{
    auto session = std::unique_ptr<TcpSession>(new (std::nothrow) TcpSession(localPort, remoteAddr, false));
    if (!session) {
        return {};
    }

    session->set_state<TcpStateSynReceived>(seq);

    return std::move(session);
}

TcpSession::~TcpSession() {
    this->close();
}

bool TcpSession::send_ack_push(const void* buffer, size_t size) {
    auto frameSize = MIN(size, 1400);
    auto* nbuf = this->tcp_alloc_nbuf(TCP_FLAGS_ACK | TCP_FLAGS_PUSH, frameSize);
    if (!nbuf) {
        return false;
    }

    auto* hdr = tcp_hdr(nbuf);

    // change this when OPTIONS support is added
    memcpy(hdr->data, buffer, frameSize);

    kprintf("[tcp] sending PUSH/ACK\n");
    return this->tcp_transmit(nbuf);
}

bool TcpSession::send_rst(NetworkBuffer* packetNbuf) {
    auto* rstNbuf = this->tcp_alloc_nbuf(TCP_FLAGS_ACK | TCP_FLAGS_RESET, 0);
    if (!rstNbuf) {
        return false;
    }

    auto* rst_hdr = tcp_hdr(rstNbuf);

    if (packetNbuf) {
        auto* pkt_hdr = tcp_hdr(packetNbuf);
        rst_hdr->seq = 0;
        rst_hdr->ack_seq = pkt_hdr->seq + 1;
    }

    kprintf("[tcp] sending RST\n");
    return this->tcp_transmit(rstNbuf);
}

bool TcpSession::send_syn(void) {
    auto* nbuf = this->tcp_alloc_nbuf(TCP_FLAGS_SYN, 0);
    if (!nbuf) {
        return false;
    }

    kprintf("[tcp] sending syn\n");
    return this->tcp_transmit(nbuf);
}

bool TcpSession::send_ack() {
    auto* ackNbuf = this->tcp_alloc_nbuf(TCP_FLAGS_ACK, 0);
    if (!ackNbuf) {
        return false;
    }

    kprintf("[tcp] sending ACK\n");
    return this->tcp_transmit(ackNbuf);
}

bool TcpSession::send_syn_ack(void) {
    auto* synackNbuf = this->tcp_alloc_nbuf(TCP_FLAGS_SYN | TCP_FLAGS_ACK, 0);
    if (!synackNbuf) {
        return false;
    }

    kprintf("[tcp] sending SYN/ACK\n");
    return this->tcp_transmit(synackNbuf);
}

bool TcpSession::send_fin(void) {
    auto* finNbuf = this->tcp_alloc_nbuf(TCP_FLAGS_FIN | TCP_FLAGS_ACK, 0);
    if (!finNbuf) {
        return false;
    }

    kprintf("[tcp] sending FIN\n");
    return this->tcp_transmit(finNbuf);
}

NetworkBuffer* TcpSession::tcp_alloc_nbuf(int flags, size_t data_size) {
    auto tcp_size = sizeof(tcp_t) + data_size;
    auto* nbuf = ip_alloc_nbuf(_remoteAddr.address, 64, IPPROTO_TCP, tcp_size);
    if (!nbuf) {
        return nullptr;
    }

    tcp_t* tcp = tcp_hdr(nbuf);
    memset(tcp, 0, sizeof(*tcp));

    tcp->sport = htons(_localPort);
    tcp->dport = htons(_remoteAddr.port);
    tcp->seq = htonl(_seq.seq());
    tcp->ack_seq = htonl(_receive_next_ack);
    tcp->hl = sizeof(tcp_t) / sizeof(uint32_t);
    tcp->rsvd = 0;
    tcp->win = htons(_receive_window);
    tcp->csum = 0;
    tcp->flags = flags;

    _seq.advance(data_size);

    if (tcp->flags & TCP_FLAGS_SYN) {
        _seq.advance(1);
    }

    if (tcp->flags & TCP_FLAGS_FIN) {
        _seq.advance(1);
    }

    return nbuf;
}

bool TcpSession::tcp_transmit(NetworkBuffer* nbuf) {
    if (!nbuf) {
        return false;
    }

    tcp_t* tcp = tcp_hdr(nbuf);
    if (0 == tcp->csum) {
        tcp->csum = tcp_checksum(nbuf);
    }

    return 0 == ip_output(nbuf);
}

bool TcpSession::process_in_segment(NetworkBuffer* nbuf) {
    __unused
    iphdr_t* ip = ip_hdr(nbuf);
    tcp_t* tcp = tcp_hdr(nbuf);

#if 0
    #define BOOL(x) ((x) ? "true" : "false")
    #define PBOOL(x) kprintf("[tcp] " SX(x) " = %s\n", BOOL((x)))

    bool is_ack = tcp_is_ack(tcp);
    bool is_syn = tcp_is_syn(tcp);
    bool is_syn_ack = tcp_is_synack(tcp);
    bool is_rst = tcp_is_rst(tcp);
    bool is_fin = tcp_is_fin(tcp);
    bool is_seq_match = ntohl(tcp->ack_seq) == (_seq.seq());

    PBOOL(is_ack);
    PBOOL(is_syn);
    PBOOL(is_syn_ack);
    PBOOL(is_rst);
    PBOOL(is_fin);
    PBOOL(is_seq_match);

    if (!is_seq_match) {
        kprintf("expected sequence %d but got %d\n", ntohl(tcp->ack_seq), _seq.seq());
    }
#endif

    _state->handle_incoming_segment(nbuf, ip, tcp);

    return false;
}

void TcpSession::close(void) {
    // TODO: implement properly..

    _receive_next_ack = 0;
    _receive_window = 0;

    if (0 != _localPort && _releasePortOnClose) {
        releasePort(_localPort);
    }

    _localPort = 0;
    _remoteAddr = {};
}

void TcpSession::set_state(std::unique_ptr<TcpState> newState) {
    bool notifyWaitingThreads = false;

    if (_state && newState) {
        kprintf("[tcp] state %s --> %s\n", tcp_state(_state->state_enum()), tcp_state(newState->state_enum()));
    }

    if (_state && _state->state_enum() == TcpStateEnum::Established) {
        notifyWaitingThreads = true;
    }

    _state = std::move(newState);
    _stateChanged.emit(_state->state_enum());

    if (_state->has_second_stage()) {
        _state->handle_second_stage();
    }

    if (notifyWaitingThreads) {
        _inputBuffer.eventStream.emit(0);
        _outputBuffer.eventStream.emit(0);
    }
}

TcpStateEnum TcpSession::wait_state_changed(void) {
    return _stateChanged.get();
}

TcpStateEnum TcpSession::state(void) const {
    return _state->state_enum();
}

bool TcpSession::is_seq_match(const tcp_t* tcp) const {
    return ntohl(tcp->ack_seq) == (_seq.seq());
}

TcpSequence& TcpSession::seq(void) {
    return _seq;
}

uint32_t& TcpSession::receive_next_ack(void) {
    return _receive_next_ack;
}

uint16_t& TcpSession::receive_window(void) {
    return _receive_window;
}

// TODO: limit push function to window size??
int TcpSession::push_input_bytes(const uint8_t* buffer, size_t size) {
    InterruptsMutex guard(true);

    std::copy_n(buffer, size, std::back_inserter(_inputBuffer.buffer));

    _receive_next_ack += size;

    _inputBuffer.eventStream.emit(size);

    return size;
}

int TcpSession::push_output_bytes(const uint8_t* buffer, size_t size) {
    {
        InterruptsMutex guard(true);
        std::copy_n(buffer, size, std::back_inserter(_outputBuffer.buffer));
    }

    _outputBuffer.eventStream.emit(size);

    _state->handle_output_trigger();
    return size;
}

int TcpSession::pop_output_bytes(uint8_t* buffer, size_t size, bool wait) {
    InterruptsMutex guard(true);

    while (0 == _outputBuffer.buffer.size() && wait) {
        guard.unlock();
        _outputBuffer.eventStream.get();
        guard.lock();
    }

    size_t avail_size = MIN(_outputBuffer.buffer.size(), size);
    if (0 == avail_size) {
        return 0;
    }

    kprintf("pop_output_bytes(%p, %d, %s): avail_size=%d)\n", buffer, size, wait ? "true" : "false", avail_size);

    std::copy_n(_outputBuffer.buffer.begin(), avail_size, buffer);
    _outputBuffer.buffer.erase(_outputBuffer.buffer.begin(), _outputBuffer.buffer.begin() + avail_size);

    return avail_size;
}

int TcpSession::pop_input_bytes(uint8_t* buffer, size_t size, bool wait) {
    if (TcpStateEnum::Closed == this->state()) {
        return 0;
    }

    if (TcpStateEnum::Established != this->state()) {
        return -1;
    }

    InterruptsMutex guard(true);
    while (0 == _inputBuffer.buffer.size() && wait) {
        if (TcpStateEnum::Closed == this->state()) {
            return 0;
        }

        if (TcpStateEnum::Established != this->state()) {
            return -1;
        }

        guard.unlock();
        _inputBuffer.eventStream.get();
        guard.lock();
    }

    size_t avail_size = MIN(_inputBuffer.buffer.size(), size);

    kprintf("pop_input_bytes(%p, %d, %s): avail_size=%d)\n", buffer, size, wait ? "true" : "false", avail_size);

    std::copy_n(_inputBuffer.buffer.begin(), avail_size, buffer);
    _inputBuffer.buffer.erase(_inputBuffer.buffer.begin(), _inputBuffer.buffer.begin() + avail_size);

    return avail_size;
}

uint16_t TcpSession::localPort(void) const {
    return _localPort;
}

SocketAddress TcpSession::remoteAddr(void) const {
    return _remoteAddr;
}

int TcpSession::acceptNewClient(void) {
    if (this->state() != TcpStateEnum::Listening) {
        return -1;
    }

    TcpStateListening* tcpStateListening = (TcpStateListening*)_state.get();
    return tcpStateListening->getNewClientFd();
}
