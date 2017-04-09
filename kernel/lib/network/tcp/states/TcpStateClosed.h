#ifndef GZOS_TCPSTATECLOSED_H
#define GZOS_TCPSTATECLOSED_H
#ifdef __cplusplus

#include "lib/network/tcp/states/TcpState.h"

class TcpStateClosed : public TcpState
{
public:
    TcpStateClosed(TcpSession& session);
    virtual ~TcpStateClosed(void) = default;

    virtual void handle_incoming_segment(NetworkBuffer* nbuf, const iphdr_t* ip, const tcp_t* tcp) override;
};

#endif //cplusplus
#endif //GZOS_TCPSTATECLOSED_H
