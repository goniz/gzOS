#include <lib/primitives/SuspendableMutex.h>
#include <lib/primitives/hashmap.h>
#include <lib/primitives/LockGuard.h>
#include <algorithm>
#include <set>
#include <platform/kprintf.h>
#include <platform/clock.h>
#include <sys/param.h>
#include "lib/network/tcp/tcp_socket.h"
#include "lib/network/tcp/tcp_sessions.h"

static SuspendableMutex _tcpLock;
static std::set<uint16_t> _tcpPorts;
static std::vector<TcpFileDescriptor*> _tcpSessions;

void registerTcpDescriptor(TcpFileDescriptor* fileDescriptor) {
    LockGuard guard(_tcpLock);
    _tcpSessions.push_back(fileDescriptor);
}

void removeTcpDescriptor(TcpFileDescriptor* fileDescriptor) {
    LockGuard guard(_tcpLock);

    auto iter = std::find(_tcpSessions.begin(), _tcpSessions.end(), fileDescriptor);
    if (_tcpSessions.end() == iter) {
        return;
    }

    _tcpSessions.erase(iter);
}

TcpFileDescriptor* getTcpDescriptorByFourTuple(const SocketAddress& localEP, const SocketAddress& remoteEP) {
//    kprintf("[tcp] Searching for socket for %08x:%d --> %08x:%d\n", remoteEP.address, remoteEP.port, localEP.address, localEP.port);

    auto activeSessionComparator = [&localEP, &remoteEP](const TcpFileDescriptor* session) -> bool {
        const auto& remote = session->remote_address();

//        kprintf("[tcp] Comparing against socket %d --> %08x:%d\n",
//                session->local_port(),
//                remote.address, remote.port);
        return session->local_port() == localEP.port && remote == remoteEP;
    };

    LockGuard guard(_tcpLock);
    auto session = std::find_if(_tcpSessions.begin(), _tcpSessions.end(), activeSessionComparator);
    if (_tcpSessions.end() != session) {
//        kprintf("[tcp] Found connection fd %p\n", *session);
        return *session;
    }

    auto listeningSessionComparator = [&localEP](const TcpFileDescriptor* session) -> bool {
//        kprintf("[tcp] Comparing against socket %d --> %08x:%d\n",
//                session->local_port(),
//                localEP.address, localEP.port);
//        kprintf("[tcp] is listening? %s\n", session->is_listening() ? "yes" : "no");
        return session->is_listening() && session->local_port() == localEP.port;
    };

    auto listeningSession = std::find_if(_tcpSessions.begin(), _tcpSessions.end(), listeningSessionComparator);
    if (_tcpSessions.end() != listeningSession) {
//        kprintf("[tcp] Found listening fd %p\n", *listeningSession);
        return *listeningSession;
    }

    kprintf("[tcp] did not find connection\n");
    return nullptr;
}

static uint16_t _basePort = 0;
uint16_t allocatePort(uint16_t port) {
    LockGuard guard(_tcpLock);

    if (0 == _basePort) {
        srand(clock_get_raw_count());
        _basePort = MAX(10000, rand() % UINT16_MAX);
    }

    if (0 != port) {
        if (_tcpPorts.emplace(port).second) {
            return port;
        }
    } else {

        for (int i = _basePort; i < UINT16_MAX; i++) {
            if (_tcpPorts.emplace(i).second) {
                return i;
            }
        } // for
    } // if else

    return 0;
}

bool releasePort(uint16_t port) {
    LockGuard guard(_tcpLock);

    return 0 < _tcpPorts.erase(port);
}
