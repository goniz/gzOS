#include <lib/network/tcp/TcpSession.h>
#include <tcp/tcp_socket.h>
#include <lib/kernel/vfs/vfs_api.h>
#include <lib/syscall/syscall.h>
#include "lib/network/tcp/states/TcpStateListening.h"

TcpStateListening::TcpStateListening(TcpSession& session, int backlog)
    : TcpState(TcpStateEnum::Listening, session),
      _acceptQueue(backlog)
{

}

void TcpStateListening::handle_incoming_segment(NetworkBuffer* nbuf, const iphdr_t* ip, const tcp_t* tcp)
{
    if (tcp->flags != TCP_FLAGS_SYN) {
        _session.send_rst(nbuf);
        return;
    }

    SocketAddress remoteAddr {
            .address = ip_src(ip),
            .port = tcp_src_port(tcp)
    };

    auto newfd = TcpFileDescriptor::createListeningDescriptor(_session.localPort(), remoteAddr, tcp_seq(tcp));
    _acceptQueue.push(std::move(newfd), false);
}

int TcpStateListening::getNewClientFd(bool wait)
{
    while (true) {
        std::unique_ptr<TcpFileDescriptor> newClientFd(nullptr);
        if (!_acceptQueue.pop(newClientFd, wait)) {
            if (wait) {
                continue;
            } else {
                return -1;
            }
        }

        if (!newClientFd) {
            continue;
        }

        if (!newClientFd->wait_connected()) {
            continue;
        }

        return vfs_pushfd(std::move(newClientFd));
    }
}

bool TcpStateListening::hasNewClientsInQueue(void) const {
    return !_acceptQueue.empty();
}
