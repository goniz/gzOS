#include <sys/param.h>
#include <checksum.h>
#include <platform/drivers.h>
#include <lib/primitives/hexdump.h>
#include "lib/network/tcp/tcp.h"
#include "lib/network/tcp/TcpSession.h"
#include "tcp_sessions.h"
#include "TcpSession.h"
#include "tcp.h"
#include "tcp_out_segment.h"

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

TcpSession::TcpSession(uint16_t localPort, SocketAddress remoteAddr, bool releasePortOnClose)
        : _state(TcpState::Closed),
          _localPort(localPort),
          _remoteAddr(remoteAddr),
          _seq(),
          _receive_next_ack(0),
          _receive_window(29200),
          _releasePortOnClose(releasePortOnClose),
          _bytes_in(150),
          _bytes_out(150),
          _segment_inflight(nullptr)
{
    kprintf("TcpSession this %p\n", this);

}

std::unique_ptr<TcpSession> TcpSession::createTcpClient(uint16_t localPort, SocketAddress remoteAddr) {
    auto session = std::unique_ptr<TcpSession>(new (std::nothrow) TcpSession(localPort, remoteAddr, true));
    if (!session) {
        return {};
    }

    session->set_state(TcpState::SynSent);
    session->send_syn();

    return std::move(session);
}

std::unique_ptr<TcpSession> TcpSession::createTcpRejector(uint16_t localPort, SocketAddress remoteAddr) {
    return std::unique_ptr<TcpSession>(new (std::nothrow) TcpSession(localPort, remoteAddr, false));
}

TcpSession::~TcpSession() {
    this->close();
}

bool TcpSession::queue_out_bytes(NetworkBuffer* nbuf) {
    if (_state != TcpState::Established) {
        return false;
    }

    if (!_bytes_out.push(nbuf)) {
        return false;
    }

    this->pop_bytes_out();
    return true;
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

bool TcpSession::send_syn(void) {
    auto* nbuf = this->tcp_alloc_nbuf(TCP_FLAGS_SYN, 0);
    if (!nbuf) {
        return false;
    }

    kprintf("[tcp] sending syn\n");
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

bool TcpSession::send_ack() {
    auto* ackNbuf = this->tcp_alloc_nbuf(TCP_FLAGS_ACK, 0);
    if (!ackNbuf) {
        return false;
    }

    kprintf("[tcp] sending ACK\n");
    return this->tcp_transmit(ackNbuf);
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
    if (tcp->flags == TCP_FLAGS_SYN) {
        _seq.advance(1);
    }

    return nbuf;
}

bool TcpSession::tcp_transmit(NetworkBuffer* nbuf) {
    if (!nbuf) {
        return false;
    }

    return nullptr != (_segment_inflight = tcp_out_queue_packet(nbuf, (TcpOutSegmentErrorFunc)TcpSession::handle_out_segment_error, this));
}

#define BOOL(x) ((x) ? "true" : "false")
#define PBOOL(x) kprintf("[tcp] " SX(x) " = %s\n", BOOL((x)))

void TcpSession::reset_inflight_segment() {
    InterruptsMutex guard(true);

    if (_segment_inflight) {

        _segment_inflight->valid = false;

        if (_segment_inflight->retransmissionTimer) {
            _segment_inflight->retransmissionTimer->stop();
        }

        _segment_inflight.reset();
    }
}

bool TcpSession::process_in_segment(NetworkBuffer* nbuf) {
    __unused
    iphdr_t* ip = ip_hdr(nbuf);
    tcp_t* tcp = tcp_hdr(nbuf);

    bool is_ack = tcp->flags & TCP_FLAGS_ACK;
    bool is_syn = tcp->flags & TCP_FLAGS_SYN;
    bool is_syn_ack = is_ack && is_syn;
    bool is_rst = tcp->flags & TCP_FLAGS_RESET;
    bool is_fin = tcp->flags & TCP_FLAGS_FIN;
    bool is_seq_match = ntohl(tcp->ack_seq) == (_seq.seq());

    PBOOL(is_ack);
    PBOOL(is_syn);
    PBOOL(is_syn_ack);
    PBOOL(is_rst);
    PBOOL(is_fin);
    PBOOL(is_seq_match);

    switch (_state)
    {
        case TcpState::SynSent: {
            if (is_seq_match && is_rst) {
                this->set_state(TcpState::Closed);
                break;
            }

            if (!is_syn_ack || !is_seq_match) {
                this->send_rst();
                this->close();
                break;
            }

            _receive_next_ack = ntohl(tcp->seq) + 1;

            this->reset_inflight_segment();

            this->send_ack();
            this->set_state(TcpState::Established);

            this->pop_bytes_out();
            goto free_nbuf;
        }

        case TcpState::Established: {
            if (is_seq_match && is_fin) {
                this->reset_inflight_segment();
                this->send_ack();
                this->set_state(TcpState::CloseWait);
                _bytes_out.notifyAll(0);
                _bytes_in.notifyAll(0);
                goto free_nbuf;
            }

            if (!is_ack || !is_seq_match) {
                goto free_nbuf;
            }

            this->reset_inflight_segment();

            auto data_size = tcp_data_length(ip, tcp);
            if (data_size != 0) {
                auto* bytes_in_nbuf = nbuf_clone_offset_nometadata(nbuf, nbuf_offset(nbuf, tcp->data));
                assert(NULL != bytes_in_nbuf);
                auto result = _bytes_in.push(bytes_in_nbuf, false);
                assert(true == result);
                _receive_next_ack += data_size;
                this->send_ack();
            }

            this->pop_bytes_out();

            goto free_nbuf;
        }

        case TcpState::CloseWait:
            goto free_nbuf;

        case TcpState::Closed:
            this->send_rst(nbuf);
            goto free_nbuf;

        default:
            goto free_nbuf;
    }

free_nbuf:
    return false;

//keep_nbuf:
    return true;
}

void TcpSession::close(void) {
    // TODO: implement FIN
    if (TcpState::Closed != _state) {
        this->send_rst(NULL);
    }

    this->set_state(TcpState::Closed);
    _receive_next_ack = 0;
    _receive_window = 0;

    if (0 != _localPort && _releasePortOnClose) {
        releasePort(_localPort);
    }

    _localPort = 0;
    _remoteAddr = {};
}

void TcpSession::set_state(TcpState newState) {
    kprintf("[tcp] state %s --> %s\n", tcp_state(_state), tcp_state(newState));
    _state = newState;
    _stateChanged.emit(newState);
}

void TcpSession::pop_bytes_out(void) {
    kprintf("TcpSession::pop_bytes_out: %p\n", _segment_inflight.get());
    if (_segment_inflight) {
        return;
    }

    NetworkBuffer* bytes_nbuf(nullptr);
    if (!_bytes_out.pop(bytes_nbuf, false)) {
        return;
    }

    if (!nbuf_is_valid(bytes_nbuf)) {
        return;
    }

    this->send_ack_push(nbuf_data(bytes_nbuf), nbuf_size(bytes_nbuf));

    nbuf_free(bytes_nbuf);
}

int TcpSession::pop_bytes_in(void* buffer, size_t size) {
    if (TcpState::Closed == _state) {
        return 0;
    }

    if (TcpState::Established != _state) {
        return -1;
    }

    NetworkBuffer* nbuf(nullptr);
    if (!_bytes_in.pop(nbuf, true)) {
        // NOTE: should get here only if we got EOF (?)
        return 0;
    }

    auto size_to_copy = MIN(size, nbuf_size(nbuf));
    const uint8_t* nbuf_pos = (const uint8_t*) nbuf_data(nbuf);
    memcpy(buffer, nbuf_pos, size_to_copy);

    if (size_to_copy != nbuf_size(nbuf)) {
        auto* newNbuf = nbuf_clone_offset_nometadata(nbuf, size_to_copy - 1);
        assert(newNbuf != nullptr);

        _bytes_in.push_head(newNbuf);
    }

    nbuf_free(nbuf);

    return size_to_copy;
}

void TcpSession::handle_out_segment_error(std::shared_ptr<TcpOutSegment> segment, TcpSession* self) {
    InterruptsMutex guard(true);

    if (segment != self->_segment_inflight) {
        return;
    }

    self->_segment_inflight.reset();
    self->close();
}

TcpState TcpSession::wait_state_changed(void) {
    return _stateChanged.get();
}

TcpState TcpSession::state(void) const {
    return _state;
}

static const char* _states[] = {
    "Closed",
    "Syn-Sent",
    "Established",
    "Listen",
    "Close-Wait"
};
const char* tcp_state(TcpState state) {
    return _states[int(state)];
}
