#ifndef GZOS_TCPFILEDESCRIPTOR_H
#define GZOS_TCPFILEDESCRIPTOR_H
#ifdef __cplusplus

#include <memory>
#include <lib/primitives/Event.h>
#include <lib/primitives/Timer.h>
#include "lib/network/socket.h"
#include "lib/network/tcp/TcpSequence.h"
#include <lib/network/tcp/TcpSession.h>

class TcpFileDescriptor : public SocketFileDescriptor
{
public:
    TcpFileDescriptor(void);
    TcpFileDescriptor(uint16_t localPort, SocketAddress remoteAddr);
    virtual ~TcpFileDescriptor(void);

    virtual int bind(const SocketAddress& addr) override;
    virtual int connect(const SocketAddress& addr) override;

    virtual int read(void* buffer, size_t size) override;
    virtual int write(const void* buffer, size_t size) override;
    virtual int seek(int where, int whence) override;
    virtual void close(void) override;

    bool process_in_segment(NetworkBuffer* nbuf);
    bool is_listening(void) const;
    bool is_connected(void) const;

    const SocketAddress& remote_address(void) const {
        return _remoteAddr;
    }

    uint16_t local_port(void) const {
        return _localPort;
    }

private:
    uint16_t                    _localPort;
    SocketAddress               _remoteAddr;
    std::unique_ptr<TcpSession> _session;
};

#endif //cplusplus
#endif //GZOS_TCPFILEDESCRIPTOR_H
