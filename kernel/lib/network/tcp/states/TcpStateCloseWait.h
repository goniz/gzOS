#ifndef GZOS_TCPSTATECLOSEWAIT_H
#define GZOS_TCPSTATECLOSEWAIT_H

#include "lib/network/tcp/states/TcpState.h"

#ifdef __cplusplus

class TcpStateCloseWait : public TcpState
{
public:
    TcpStateCloseWait(TcpSession& session);
    virtual ~TcpStateCloseWait(void) = default;

    virtual bool has_second_stage(void) const override;
    virtual void handle_second_stage(void) override;

    virtual void handle_incoming_segment(NetworkBuffer* nbuf, const iphdr_t* ip, const tcp_t* tcp) override;
};

#endif //cplusplus
#endif //GZOS_TCPSTATECLOSEWAIT_H
