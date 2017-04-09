#include "lib/network/tcp/states/TcpStateClosed.h"
#include <lib/network/tcp/TcpSession.h>

TcpStateClosed::TcpStateClosed(TcpSession& session)
    : TcpState(TcpStateEnum::Closed, session)
{

}

void TcpStateClosed::handle_incoming_segment(NetworkBuffer* nbuf, const iphdr_t* ip, const tcp_t* tcp)
{
    _session.send_rst(nbuf);
}
