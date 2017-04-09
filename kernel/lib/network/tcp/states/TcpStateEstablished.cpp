#include <algorithm>
#include <lib/network/nbuf.h>
#include <lib/network/tcp/tcp.h>
#include "lib/network/tcp/states/TcpStateEstablished.h"
#include "lib/network/tcp/states/TcpStateCloseWait.h"
#include "lib/network/tcp/states/TcpStateLastAck.h"
#include <lib/network/tcp/TcpSession.h>

TcpStateEstablished::TcpStateEstablished(TcpSession& session)
    : TcpState(TcpStateEnum::Established, session),
      _waiting_for_ack(false)
{
    this->handle_output_trigger();
}

void TcpStateEstablished::handle_incoming_segment(NetworkBuffer* nbuf, const iphdr_t* ip, const tcp_t* tcp)
{
    if (!_session.is_seq_match(tcp)) {
        return;
    }

    if (_session.is_seq_match(tcp) && tcp_is_fin(tcp)) {
        _session.set_state<TcpStateCloseWait>();
        return;
    }

//    if (!tcp_is_ack(tcp)) {
//        this->handle_output_trigger();
//        return;
//    }

    _waiting_for_ack = false;

    auto data_size = tcp_data_length(ip, tcp);
    if (0 == data_size) {
        this->handle_output_trigger();
        return;
    }

    _session.push_input_bytes(tcp->data, data_size);
    _session.send_ack();

    this->handle_output_trigger();
}

void TcpStateEstablished::handle_output_trigger(void)
{
    InterruptsMutex guard(true);

    if (_waiting_for_ack) {
        return;
    }

    uint8_t buffer[1400];
    size_t output_bytes = _session.pop_output_bytes(buffer, sizeof(buffer), false);
    if (0 == output_bytes) {
        return;
    }

    _waiting_for_ack = true;
    _session.send_ack_push(buffer, output_bytes);
}
