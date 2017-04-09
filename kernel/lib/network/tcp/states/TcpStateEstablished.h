#ifndef GZOS_TCPSTATEESTABLISHED_H
#define GZOS_TCPSTATEESTABLISHED_H

#include "lib/network/tcp/states/TcpState.h"

#ifdef __cplusplus

class TcpStateEstablished : public TcpState
{
public:
    TcpStateEstablished(TcpSession& session);
    virtual ~TcpStateEstablished(void) = default;

    virtual void handle_incoming_segment(NetworkBuffer* nbuf, const iphdr_t* ip, const tcp_t* tcp) override;

    void handle_output_trigger(void) override;

private:
    bool _waiting_for_ack;

};

#endif //cplusplus
#endif //GZOS_TCPSTATEESTABLISHED_H
