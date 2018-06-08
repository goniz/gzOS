#ifndef GZOS_TCPSTATEFINSENT_H
#define GZOS_TCPSTATEFINSENT_H

#include "TcpState.h"

#ifdef __cplusplus


class TcpStateFinSent : public TcpState {
public:
    TcpStateFinSent(TcpSession& session);
    virtual ~TcpStateFinSent() = default;

    virtual void handle_incoming_segment(NetworkBuffer* nbuf, const iphdr_t* ip, const tcp_t* tcp) override;

    virtual bool has_second_stage(void) const override;
    virtual void handle_second_stage(void) override;
};


#endif //cplusplus

#endif //GZOS_TCPSTATEFINSENT_H
