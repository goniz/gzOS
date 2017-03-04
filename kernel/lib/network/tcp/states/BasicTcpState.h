#ifndef GZOS_TCPSTATE_H
#define GZOS_TCPSTATE_H

#include <lib/network/tcp/TcpSession.h>

#ifdef __cplusplus

class BasicTcpState
{
public:
    BasicTcpState(TcpSession& session);

protected:
    TcpSession& _session;
};


#endif //cplusplus
#endif //GZOS_TCPSTATE_H
