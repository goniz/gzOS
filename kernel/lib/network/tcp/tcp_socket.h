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
    static std::unique_ptr<TcpFileDescriptor> createPlainDescriptor(void);
    static std::unique_ptr<TcpFileDescriptor> createRejectingDescriptor(uint16_t localPort, SocketAddress remoteAddr);
    static std::unique_ptr<TcpFileDescriptor> createListeningDescriptor(uint16_t localPort, SocketAddress remoteAddr,
                                                                        uint32_t seq);
    virtual ~TcpFileDescriptor(void);

    virtual const char* type(void) const override;
    virtual int bind(const SocketAddress& addr) override;
    virtual int listen(int backlog) override;
    virtual int accept(SocketAddress* clientAddress, size_t* clientAddressLen) override;
    virtual int connect(const SocketAddress& addr) override;
    virtual int read(void* buffer, size_t size) override;
    virtual int recvfrom(void* buffer, size_t size, SocketAddress* address) override;
    virtual int write(const void* buffer, size_t size) override;
    virtual int poll(bool* read_ready, bool* write_ready) override;
    virtual int set_blocking(int blocking_state) override;

    int sendto(const void* buffer, size_t size, const SocketAddress& address) override;

    virtual int seek(int where, int whence) override;
    virtual void close(void) override;

    bool process_in_segment(NetworkBuffer* nbuf);
    bool is_listening(void) const;
    bool is_connected(void) const;
    bool wait_connected(void) const;

    const SocketAddress& remote_address(void) const {
        return _remoteAddr;
    }

    uint16_t local_port(void) const {
        return _localPort;
    }

private:
    TcpFileDescriptor(void);
    TcpFileDescriptor(uint16_t localPort, SocketAddress remoteAddr);
    TcpFileDescriptor(uint16_t localPort, SocketAddress remoteAddr, uint32_t seq);

    uint16_t                    _localPort;
    SocketAddress               _remoteAddr;
    std::unique_ptr<TcpSession> _session;
};

#endif //cplusplus
#endif //GZOS_TCPFILEDESCRIPTOR_H
