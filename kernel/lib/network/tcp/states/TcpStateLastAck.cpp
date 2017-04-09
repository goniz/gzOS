#include <lib/network/tcp/TcpSession.h>
#include "lib/network/tcp/states/TcpStateLastAck.h"
#include "lib/network/tcp/states/TcpStateClosed.h"

TcpStateLastAck::TcpStateLastAck(TcpSession& session)
    : TcpState(TcpStateEnum::LastAck, session)
{

}

void TcpStateLastAck::handle_incoming_segment(NetworkBuffer* nbuf, const iphdr_t* ip, const tcp_t* tcp)
{
    if (!tcp_is_ack(tcp) || !_session.is_seq_match(tcp)) {
        return;
    }

    _session.send_ack();
    _session.set_state<TcpStateClosed>();
}
