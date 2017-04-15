#include <platform/drivers.h>
#include "lib/network/socket.h"
#include "lib/network/tcp/tcp_socket.h"

int tcp_in_main(void* argument);

static int tcp_proto_init(void)
{
    socket_register_triple({AF_INET, SOCK_STREAM, IPPROTO_TCP}, []() {
        return std::unique_ptr<SocketFileDescriptor>(TcpFileDescriptor::createPlainDescriptor());
    });

    Scheduler::instance().createKernelThread("TcpInput", tcp_in_main, nullptr, 8192);
    return 0;
}

DECLARE_DRIVER(tcp_proto, tcp_proto_init, STAGE_SECOND + 1);