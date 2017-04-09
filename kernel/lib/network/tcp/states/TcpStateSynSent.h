#ifndef GZOS_TCPSTATESYNSENT_H
#define GZOS_TCPSTATESYNSENT_H

#include <lib/network/tcp/TcpSession.h>
#include <lib/network/tcp/states/TcpState.h>

#ifdef __cplusplus

class TcpStateSynSent : public TcpState
{
public:
    TcpStateSynSent(TcpSession& session);
    virtual ~TcpStateSynSent(void) = default;

    virtual void handle_incoming_segment(NetworkBuffer* nbuf, const iphdr_t* ip, const tcp_t* tcp) override;

private:
    bool send_syn(void);
};


#endif //cplusplus
#endif //GZOS_TCPSTATESYNSENT_H
