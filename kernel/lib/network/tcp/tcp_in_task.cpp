#include <lib/syscall/syscall.h>
#include <lib/primitives/hexdump.h>
#include <lib/network/nbuf.h>
#include <lib/network/ip/ip.h>
#include <lib/network/socket.h>
#include "lib/network/checksum.h"
#include "lib/network/tcp/tcp.h"
#include "lib/network/tcp/tcp_socket.h"
#include "lib/network/tcp/tcp_sessions.h"


static bool tcp_validate_packet(const NetworkBuffer* nbuf, const tcp_t* tcp);
static basic_queue<NetworkBuffer*> gInSegments(150);

__attribute__((noreturn))
int tcp_in_main(void* argument) {
    syscall(SYS_NR_SET_THREAD_RESPONSIVE, 1);

    while (1) {
        NetworkBuffer* packet = NULL;
        if (!gInSegments.pop(packet, true)) {
            continue;
        }

        TcpFileDescriptor* session = nullptr;
        std::unique_ptr<TcpFileDescriptor> rstFd(nullptr);

        auto* ip = ip_hdr(packet);
        auto* tcp = tcp_hdr(packet);

        SocketAddress localEndpoint{ntohl(ip->daddr), ntohs(tcp->dport)};
        SocketAddress remoteEndpoint{ntohl(ip->saddr), ntohs(tcp->sport)};

        session = getTcpDescriptorByFourTuple(localEndpoint, remoteEndpoint);
        if (nullptr == session) {
            rstFd = TcpFileDescriptor::createRejectingDescriptor(localEndpoint.port, remoteEndpoint);
            session = rstFd.get();
        }

        if (!session->process_in_segment(packet)) {
            nbuf_free(packet);
        }
    }
}

int tcp_input(NetworkBuffer* packet) {
    int ret = 0;
    auto* tcp = tcp_hdr(packet);

    if (!tcp_validate_packet(packet, tcp)) {
        goto error;
    }

    if (!gInSegments.push(packet, false)) {
        goto error;
    }

    goto exit;

error:
    ret = -1;
    nbuf_free(packet);

exit:
    return ret;
}

static bool tcp_validate_packet(const NetworkBuffer* nbuf, const tcp_t* tcp)
{
    iphdr_t* ip = ip_hdr(nbuf);

    if (nbuf_size_from(nbuf, tcp) < sizeof(tcp_t)) {
        kprintf("tcp: packet is too small, dropping. (buf %d tcp %d)\n", nbuf_size_from(nbuf, tcp), sizeof(tcp_t));
        return false;
    }

    if (nbuf_size_from(nbuf, tcp->data) < tcp_data_length(ip, tcp)) {
        kprintf("tcp: packet buffer is too small, header length corrupted? (buf %d tcp_data_length(ip, tcp) %d)\n",
                nbuf_size_from(nbuf, tcp->data),
                tcp_data_length(ip, tcp));
        return false;
    }

    const auto expected = tcp_checksum(nbuf);
    const auto found = ntohs(tcp->csum);
    if (found != expected) {
        kprintf("tcp: invalid checksum - expected %04x found %04x\n", expected, found);

        const iphdr_t* ip = ip_hdr(nbuf);
        hexDump(NULL, (void*)ip, ntohs(ip->len));
        return false;
    }

    return true;
}