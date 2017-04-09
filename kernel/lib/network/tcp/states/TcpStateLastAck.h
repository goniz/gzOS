#ifndef GZOS_TCPSTATELASTACK_H
#define GZOS_TCPSTATELASTACK_H

#include "lib/network/tcp/states/TcpState.h"

#ifdef __cplusplus

class TcpStateLastAck : public TcpState
{
public:
    TcpStateLastAck(TcpSession& session);
    virtual ~TcpStateLastAck(void) = default;

    virtual void handle_incoming_segment(NetworkBuffer* nbuf, const iphdr_t* ip, const tcp_t* tcp) override;
};

#endif //cplusplus
#endif //GZOS_TCPSTATELASTACK_H
