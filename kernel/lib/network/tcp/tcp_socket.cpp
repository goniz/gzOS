#include <lib/primitives/interrupts_mutex.h>
#include <lib/primitives/hashmap.h>
#include <lib/primitives/spinlock_mutex.h>
#include <platform/kprintf.h>
#include <lib/kernel/proc/Scheduler.h>
#include <lib/primitives/Timer.h>
#include <sys/param.h>
#include "lib/network/tcp/tcp_socket.h"
#include "lib/network/tcp/tcp_sessions.h"
#include "lib/network/tcp/tcp.h"
#include "lib/network/checksum.h"

TcpFileDescriptor::TcpFileDescriptor(void)
    : _localPort(0),
      _remoteAddr{0, 0},
      _session(nullptr)
{
    registerTcpDescriptor(this);
}

TcpFileDescriptor::TcpFileDescriptor(uint16_t localPort, SocketAddress remoteAddr)
    : _localPort(localPort),
      _remoteAddr(remoteAddr),
      _session(TcpSession::createTcpRejector(_localPort, _remoteAddr))
{
    registerTcpDescriptor(this);
}

TcpFileDescriptor::TcpFileDescriptor(uint16_t localPort, SocketAddress remoteAddr, uint32_t seq)
    : _localPort(localPort),
      _remoteAddr(remoteAddr),
      _session(TcpSession::createTcpClientAcceptor(_localPort, _remoteAddr, seq))
{
    registerTcpDescriptor(this);
}

TcpFileDescriptor::~TcpFileDescriptor(void) {
    this->close();
    removeTcpDescriptor(this);
}

int TcpFileDescriptor::read(void* buffer, size_t size) {
    if (!_session) {
        kprintf("[tcp] read(%p, %d): called without session\n", buffer, size);
        return 0;
    }

    // TODO: return error on the wrong states..
    if (!this->is_connected()) {
        kprintf("[tcp] read(%p, %d): called on a non-established socket\n", buffer, size);
        return 0;
    }

    return _session->pop_input_bytes((uint8_t*) buffer, size, true);
}

int TcpFileDescriptor::recvfrom(void* buffer, size_t size, SocketAddress* address) {
    int result = this->read(buffer, size);

    if (-1 != result && address) {
        *address = _remoteAddr;
    }

    return result;
}

int TcpFileDescriptor::write(const void* buffer, size_t size) {
    if (!_session) {
        return 0;
    }

    // TODO: return error on the wrong states..
    if (!this->is_connected()) {
        return 0;
    }

    return _session->push_output_bytes((const uint8_t*) buffer, size);
}

int TcpFileDescriptor::sendto(const void* buffer, size_t size, const SocketAddress& address) {
    if (address.address != _remoteAddr.address || address.port != _remoteAddr.port) {
        return -1;
    }

    return this->write(buffer, size);
}

int TcpFileDescriptor::seek(int where, int whence) {
    return -1;
}

void TcpFileDescriptor::close(void) {
    if (_session) {
        _session->close();
    }

    _localPort = 0;
    _remoteAddr = {};
}

int TcpFileDescriptor::bind(const SocketAddress& addr) {
    if (IPADDR_ANY != addr.address) {
        kprintf("bind: only IPADDR_ANY is supported\n");
        return -1;
    }

    auto port = allocatePort(addr.port);
    if (0 == port) {
        kprintf("bind: failed to allocate port\n");
        return -1;
    }

    this->_localPort = port;
    return 0;
}

int TcpFileDescriptor::listen(int backlog) {
    if (_session) {
        return -1;
    }

    if (0 == _localPort &&
        0 != this->bind({IPADDR_ANY, 0})) {
        kprintf("listen: failed to bind local port\n");
        return -1;
    }

    _session = TcpSession::createTcpServer(_localPort, backlog);
    if (!_session) {
        return -1;
    }

    return 0;
}

int TcpFileDescriptor::accept(SocketAddress* clientAddress, size_t* clientAddressLen) {
    if (!_session) {
        return -1;
    }

    if (!this->is_listening()) {
        return -1;
    }

    int newfd = _session->acceptNewClient();
    if (-1 == newfd) {
        return -1;
    }

    return newfd;
}

int TcpFileDescriptor::connect(const SocketAddress& addr) {
    if (_session) {
        return -1;
    }

    if (0 == addr.address || 0 == addr.port) {
        kprintf("connect: invalid arguments\n");
        return -1;
    }

    if (0 == _localPort &&
        0 != this->bind({IPADDR_ANY, 0})) {
        kprintf("connect: failed to bind local port\n");
        return -1;
    }

    _remoteAddr = addr;
    _session = TcpSession::createTcpClient(_localPort, _remoteAddr);
    if (!_session) {
        return -1;
    }

    auto new_state = _session->wait_state_changed();
    return TcpStateEnum::Established == new_state ? 0 : -1;
}

bool TcpFileDescriptor::process_in_segment(NetworkBuffer* nbuf) {
    return _session->process_in_segment(nbuf);
}

bool TcpFileDescriptor::is_listening(void) const {
    return _session && _session->state() == TcpStateEnum::Listening;
}

bool TcpFileDescriptor::is_connected() const {
    return _session && _session->state() == TcpStateEnum::Established;
}

std::unique_ptr<TcpFileDescriptor> TcpFileDescriptor::createPlainDescriptor(void) {
    return std::unique_ptr<TcpFileDescriptor>(new TcpFileDescriptor());
}

std::unique_ptr<TcpFileDescriptor> TcpFileDescriptor::createRejectingDescriptor(uint16_t localPort,
                                                                                SocketAddress remoteAddr)
{
    return std::unique_ptr<TcpFileDescriptor>(new TcpFileDescriptor(localPort, remoteAddr));
}

std::unique_ptr<TcpFileDescriptor> TcpFileDescriptor::createListeningDescriptor(uint16_t localPort,
                                                                                SocketAddress remoteAddr,
                                                                                uint32_t seq)
{
    return std::unique_ptr<TcpFileDescriptor>(new TcpFileDescriptor(localPort, remoteAddr, seq));
}
