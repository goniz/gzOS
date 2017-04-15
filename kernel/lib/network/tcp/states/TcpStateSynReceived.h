#ifndef GZOS_TCPSTATESYNRECEIVED_H
#define GZOS_TCPSTATESYNRECEIVED_H

#include <lib/network/tcp/states/TcpState.h>

#ifdef __cplusplus

class TcpStateSynReceived : public TcpState
{
public:
    TcpStateSynReceived(TcpSession& session, uint32_t seq);
    virtual ~TcpStateSynReceived(void) = default;

    virtual void handle_incoming_segment(NetworkBuffer* nbuf, const iphdr_t* ip, const tcp_t* tcp) override;

    bool has_second_stage(void) const override;
    void handle_second_stage(void) override;
};



#endif //cplusplus

#endif //GZOS_TCPSTATESYNRECEIVED_H
