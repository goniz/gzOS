#include <sys/param.h>
#include <platform/cpu.h>
#include <lib/primitives/hashmap.h>
#include "lib/network/udp/udp_socket.h"

static HashMap<int, UdpFileDescriptor*> _portSessionsMap;

UdpFileDescriptor* getUdpDescriptorByPort(uint16_t port) {
    auto** descPtr = _portSessionsMap.get(port);
    if (nullptr == descPtr) {
        return nullptr;
    }

    return *descPtr;
}

UdpFileDescriptor::UdpFileDescriptor() : sourcePort(0),
                      dstAddress{},
                      rxQueue(RX_QUEUE_SIZE)
{

}

UdpFileDescriptor::~UdpFileDescriptor(void)
{
    this->close();
}

int UdpFileDescriptor::recvfrom(void *buffer, size_t size, SocketAddress *address) {
    NetworkBuffer* nbuf = NULL;
    if ((nullptr == buffer) || (!this->rxQueue.pop(nbuf, true))) {
        return -1;
    }

    udp_t* udp = udp_hdr(nbuf);

    if (address) {
        iphdr_t* ip = ip_hdr(nbuf);
        address->address = ntohl(ip->saddr);
        address->port = ntohl(udp->sport);
    }

    size = MIN(udp_data_length(udp), size);
    memcpy(buffer, udp->data, size);

    nbuf_free(nbuf);
    return (int) size;
}

int UdpFileDescriptor::read(void *buffer, size_t size) {
    return this->recvfrom(buffer, size, nullptr);
}


int UdpFileDescriptor::sendto(const void *buffer, size_t size, const SocketAddress &address)
{
    if (0 == address.address || 0 == address.port) {
        return -1;
    }

    NetworkBuffer* nbuf = udp_alloc_nbuf(address.address, address.port, (uint16_t) size);
    if (!nbuf) {
        return -1;
    }

    udp_t* udp = udp_hdr(nbuf);

    if (0 == this->sourcePort) {
        if (-1 == this->bind({IPADDR_ANY, 0})) {
            nbuf_free(nbuf);
            return -1;
        }
    }

    udp->sport = htons(this->sourcePort);
    memcpy(udp->data, buffer, size);

    if (0 != udp_output(nbuf)) {
        return -1;
    }

    return (int) size;
}

int UdpFileDescriptor::write(const void *buffer, size_t size) {
    return this->sendto(buffer, size, this->dstAddress);
}

int UdpFileDescriptor::seek(int where, int whence) {
    return -1;
}

void UdpFileDescriptor::close(void)
{
    {
        InterruptsMutex mutex(true);
        if (0 != this->sourcePort) {
            _portSessionsMap.remove(this->sourcePort);
            this->sourcePort = 0;
            this->dstAddress = {0, 0};
        }
    }

    NetworkBuffer* nbuf = NULL;
    while (this->rxQueue.pop(nbuf, false)) {
        nbuf_free(nbuf);
    }
}

int UdpFileDescriptor::bind(const SocketAddress& addr) {
    if (IPADDR_ANY != addr.address) {
        return -1;
    }

    uint16_t port = addr.port;
    if (0 == port) {
        port = (uint16_t) (rand() % UINT16_MAX);
    }

    InterruptsMutex mutex(true);
    if (_portSessionsMap.get(port)) {
        return -1;
    }

    _portSessionsMap.put(port, std::move(this));
    this->sourcePort = port;
    return 0;
}

int UdpFileDescriptor::connect(const SocketAddress& addr) {
    if (0 == addr.address || 0 == addr.port) {
        return -1;
    }

    this->dstAddress = addr;
    return 0;
}

int UdpFileDescriptor::poll(bool* read_ready, bool* write_ready) {
    InterruptsMutex mutex(true);

    if (read_ready) {
        *read_ready = !this->rxQueue.empty();
    }

    if (write_ready) {
        *write_ready = true;
    }

    return 0;
}

const char* UdpFileDescriptor::type(void) const {
    return "UdpFileDescriptor";
}
