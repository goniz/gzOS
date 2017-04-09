#ifndef GZOS_TCPSTATE_H
#define GZOS_TCPSTATE_H

#include <string>
#include <lib/network/nbuf.h>
#include <lib/network/ip/ip.h>
#include <lib/network/tcp/tcp.h>
#include "lib/network/tcp/states/TcpStateEnum.h"

#ifdef __cplusplus

class TcpSession;
class TcpState
{
public:
    TcpState(TcpStateEnum stateEnum, TcpSession& session);
    virtual ~TcpState(void) = default;

    virtual void handle_incoming_segment(NetworkBuffer* nbuf, const iphdr_t* ip, const tcp_t* tcp) = 0;
    virtual void handle_output_trigger(void);

    virtual bool has_second_stage(void) const;
    virtual void handle_second_stage(void);

    TcpStateEnum state_enum(void) const;

protected:
    TcpStateEnum _stateEnum;
    TcpSession& _session;
};


#endif //cplusplus
#endif //GZOS_TCPSTATE_H
