#include "lib/network/tcp/TcpSession.h"
#include "lib/network/tcp/states/TcpStateSynReceived.h"
#include "TcpStateEstablished.h"

TcpStateSynReceived::TcpStateSynReceived(TcpSession& session, uint32_t seq)
    : TcpState(TcpStateEnum::SynReceived, session)
{
    _session.receive_next_ack() = seq + 1;
    _session.seq() = 0;
}

void TcpStateSynReceived::handle_incoming_segment(NetworkBuffer* nbuf, const iphdr_t* ip, const tcp_t* tcp)
{
    if (_session.is_seq_match(tcp) && tcp_is_ack(tcp)) {
        _session.set_state<TcpStateEstablished>();
    }
}

bool TcpStateSynReceived::has_second_stage(void) const {
    return true;
}

void TcpStateSynReceived::handle_second_stage(void) {
    _session.send_syn_ack();
}
