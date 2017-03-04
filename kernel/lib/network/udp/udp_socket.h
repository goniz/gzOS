#ifndef GZOS_UDP_SOCKET_H
#define GZOS_UDP_SOCKET_H

#include <lib/primitives/basic_queue.h>
#include "lib/network/nbuf.h"
#include "lib/network/socket.h"
#include "lib/network/udp/udp.h"

#ifdef __cplusplus

class UdpFileDescriptor : public SocketFileDescriptor
{
public:
    UdpFileDescriptor(void);

    virtual int read(void *buffer, size_t size) override;
    virtual int write(const void *buffer, size_t size) override;
    virtual int seek(int where, int whence) override;
    virtual void close(void) override;
    virtual int bind(const SocketAddress& addr) override;
    virtual int connect(const SocketAddress& addr) override;
    virtual int recvfrom(void *buffer, size_t size, SocketAddress *address) override;

    virtual int sendto(const void *buffer, size_t size, const SocketAddress &address) override;

private:
    friend int udp_input(NetworkBuffer *packet);

    uint16_t sourcePort;
    SocketAddress dstAddress;
    basic_queue<NetworkBuffer*> rxQueue;
};

UdpFileDescriptor* getUdpDescriptorByPort(uint16_t port);

#endif //cplusplus
#endif //GZOS_UDP_SOCKET_H
