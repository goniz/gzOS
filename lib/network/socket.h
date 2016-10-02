#ifndef GZOS_SOCKET_H
#define GZOS_SOCKET_H

#include <lib/network/ip.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    AF_INET = 2
} socket_family_t;

typedef enum {
    SOCK_STREAM = 1,
    SOCK_DGRAM,
    SOCK_RAW
} socket_type_t;

typedef struct {
    socket_family_t domain;
    socket_type_t type;
    int protocol;
} socket_triple_t;

#define IPADDR_ANY (0)

int socket(int domain, int type, int protocol);

#ifdef __cplusplus
} //extern "C"

#include <memory>
#include <lib/kernel/FileDescriptor.h>

struct SocketAddress {
    IpAddress address;
    uint16_t port;
};

class SocketFileDescriptor : public FileDescriptor {
public:
    virtual ~SocketFileDescriptor(void) = default;

    virtual int bind(const SocketAddress& addr) {
        return -1;
    }
};

typedef std::unique_ptr<SocketFileDescriptor> (*SocketFileDescriptorCreator)(void);
void socket_register_triple(socket_triple_t triple, SocketFileDescriptorCreator descriptorCreator);

SocketFileDescriptor* socket_num_to_fd(int fdnum);

#endif //cplusplus
#endif //GZOS_SOCKET_H
