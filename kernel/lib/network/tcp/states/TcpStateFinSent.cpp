#include "tcp/TcpSession.h"
#include "TcpStateFinSent.h"
#include "TcpStateLastAck.h"

TcpStateFinSent::TcpStateFinSent(TcpSession& session)
    : TcpState(TcpStateEnum::FinSent, session)
{
    session.send_fin();
}

bool TcpStateFinSent::has_second_stage(void) const {
    return true;
}

void TcpStateFinSent::handle_second_stage(void) {
    _session.set_state<TcpStateLastAck>();
}

void TcpStateFinSent::handle_incoming_segment(NetworkBuffer* nbuf, const iphdr_t* ip, const tcp_t* tcp) {

}
