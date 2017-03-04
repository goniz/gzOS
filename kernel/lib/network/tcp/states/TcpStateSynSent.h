#ifndef GZOS_TCPSTATESYNSENT_H
#define GZOS_TCPSTATESYNSENT_H

#include <lib/network/tcp/TcpSession.h>
#include <lib/network/tcp/states/BasicTcpState.h>

#ifdef __cplusplus

class TcpStateSynSent : public BasicTcpState
{
public:
    TcpStateSynSent(TcpSession& session);
};


#endif //cplusplus
#endif //GZOS_TCPSTATESYNSENT_H
