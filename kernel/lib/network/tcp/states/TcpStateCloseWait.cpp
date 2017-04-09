#include "lib/network/tcp/states/TcpStateCloseWait.h"
#include "lib/network/tcp/states/TcpStateLastAck.h"
#include <lib/network/tcp/TcpSession.h>

TcpStateCloseWait::TcpStateCloseWait(TcpSession& session)
    : TcpState(TcpStateEnum::CloseWait, session)
{
    _session.receive_next_ack()++;
    _session.send_fin();
}

void TcpStateCloseWait::handle_incoming_segment(NetworkBuffer* nbuf, const iphdr_t* ip, const tcp_t* tcp)
{

}

bool TcpStateCloseWait::has_second_stage(void) const {
    return true;
}

void TcpStateCloseWait::handle_second_stage(void) {
    _session.set_state<TcpStateLastAck>();
}
