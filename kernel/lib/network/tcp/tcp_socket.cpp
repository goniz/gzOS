#include <lib/primitives/interrupts_mutex.h>
#include <lib/primitives/hashmap.h>
#include <lib/primitives/spinlock_mutex.h>
#include <platform/kprintf.h>
#include <lib/kernel/sched/scheduler.h>
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

TcpFileDescriptor::~TcpFileDescriptor(void) {
    this->close();
    removeTcpDescriptor(this);
}

int TcpFileDescriptor::read(void* buffer, size_t size) {
    if (!_session) {
        return 0;
    }

    // TODO: return error on the wrong states..
    if (TcpState::Established != _session->state()) {
        return 0;
    }

    return _session->pop_bytes_in(buffer, size);
}

int TcpFileDescriptor::write(const void* buffer, size_t size) {
    if (!_session) {
        return 0;
    }

    // TODO: return error on the wrong states..
    if (TcpState::Established != _session->state()) {
        return 0;
    }

    auto* nbuf = nbuf_alloc(size);
    if (!nbuf) {
        return -1;
    }

    memcpy(nbuf_data(nbuf), buffer, size);
    nbuf_set_size(nbuf, size);

    if (_session->queue_out_bytes(nbuf)) {
        return size;
    } else {
        return -1;
    }
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
    return TcpState::Established == new_state ? 0 : -1;
}

bool TcpFileDescriptor::process_in_segment(NetworkBuffer* nbuf) {
    return _session->process_in_segment(nbuf);
}

bool TcpFileDescriptor::is_listening(void) const {
    return _session && _session->state() == TcpState::Listening;
}

bool TcpFileDescriptor::is_connected() const {
    return _session && _session->state() == TcpState::Established;
}
