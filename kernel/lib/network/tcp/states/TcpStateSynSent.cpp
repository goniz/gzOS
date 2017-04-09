#include <platform/kprintf.h>
#include <lib/network/tcp/tcp.h>
#include "lib/network/tcp/states/TcpStateSynSent.h"
#include "lib/network/tcp/states/TcpStateClosed.h"
#include "lib/network/tcp/states/TcpStateEstablished.h"

TcpStateSynSent::TcpStateSynSent(TcpSession& session)
    : TcpState(TcpStateEnum::SynSent, session)
{
    _session.send_syn();
}

void TcpStateSynSent::handle_incoming_segment(NetworkBuffer* nbuf, const iphdr_t* ip, const tcp_t* tcp)
{
    if (_session.is_seq_match(tcp) && tcp_is_rst(tcp)) {
        _session.set_state<TcpStateClosed>();
        return;
    }

    if (!tcp_is_synack(tcp) || !_session.is_seq_match(tcp)) {
        _session.set_state<TcpStateClosed>();
        return;
    }

    _session.receive_next_ack() = ntohl(tcp->seq) + 1;

    _session.send_ack();
    _session.set_state<TcpStateEstablished>();
}
