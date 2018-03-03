#ifndef GZOS_TCPSTATELISTENING_H
#define GZOS_TCPSTATELISTENING_H

#include "lib/primitives/basic_queue.h"
#include <memory>
#include "lib/network/tcp/tcp_socket.h"
#include "lib/network/tcp/states/TcpState.h"

#ifdef __cplusplus

class TcpStateListening : public TcpState
{
public:
    TcpStateListening(TcpSession& session, int backlog);
    virtual ~TcpStateListening(void) = default;

    virtual void handle_incoming_segment(NetworkBuffer* nbuf, const iphdr_t* ip, const tcp_t* tcp) override;

    int getNewClientFd(bool wait);
    bool hasNewClientsInQueue(void) const;

private:
    basic_queue<std::unique_ptr<TcpFileDescriptor>> _acceptQueue;
};



#endif //cplusplus

#endif //GZOS_TCPSTATELISTENING_H
