#include <lib/primitives/hashmap.h>
#include <platform/kprintf.h>
#include <platform/cpu.h>
#include <lib/primitives/basic_queue.h>
#include <sys/param.h>
#include <platform/drivers.h>
#include "udp.h"
#include "icmp.h"
#include "socket.h"

static int udp_proto_init(void);
DECLARE_DRIVER(udp_proto, udp_proto_init, STAGE_SECOND + 1);

class UdpFileDescriptor;
static HashMap<int, UdpFileDescriptor*> _portSessionsMap;

class UdpFileDescriptor : public SocketFileDescriptor
{
public:
    UdpFileDescriptor() : rxQueue(RX_QUEUE_SIZE), port(-1) { }

    virtual ~UdpFileDescriptor(void) = default;

    virtual int read(void *buffer, size_t size) override {
        NetworkBuffer* nbuf = NULL;
        if ((nullptr == buffer) || (!this->rxQueue.pop(nbuf, true))) {
            return -1;
        }

        udp_t* udp = udp_hdr(nbuf);
        size = MIN(udp_data_length(udp), size);
        memcpy(buffer, udp->data, size);
        nbuf_free(nbuf);
        return (int) size;
    }

    virtual int write(const void *buffer, size_t size) override {
        return -1;
    }

    virtual int seek(int where, int whence) override {
        return -1;
    }

    virtual void close(void) override {
        InterruptsMutex mutex;
        NetworkBuffer* nbuf = NULL;

        mutex.lock();

        while (this->rxQueue.pop(nbuf, false)) {
            nbuf_free(nbuf);
        }

        _portSessionsMap.remove(this->port);
        mutex.unlock();
    }

    virtual int bind(const SocketAddress &addr) override {
        if (IPADDR_ANY != addr.address) {
            return -1;
        }

        uint16_t port = addr.port;
        if (0 == port) {
            port = (uint16_t) (rand() % UINT16_MAX);
        }

        InterruptsMutex mutex;
        mutex.lock();
        if (_portSessionsMap.get(port)) {
            return -1;
        }

        _portSessionsMap.put(port, std::move(this));
        this->port = port;
        return 0;
    }

private:
    friend int udp_input(NetworkBuffer *packet);

    basic_queue<NetworkBuffer*> rxQueue;
    int port;
};

int udp_input(NetworkBuffer *packet)
{
    int ret = 0;
    uint16_t dport = 0;
    UdpFileDescriptor** session = nullptr;
    udp_t* udp = udp_hdr(packet);

    if (nbuf_size_from(packet, udp) < sizeof(udp_t)) {
        kprintf("udp: packet is too small, dropping. (buf %d udp %d)\n", nbuf_size_from(packet, udp), sizeof(udp_t));
        goto error;
    }

    if (nbuf_size_from(packet, udp->data) < udp_data_length(udp)) {
        kprintf("udp: packet buffer is too small, header length corrupted? (buf %d udp_data_length(udp) %d)\n",
                nbuf_size_from(packet, udp->data),
                udp_data_length(udp));
        goto error;
    }

    dport = ntohs(udp->dport);
    session = _portSessionsMap.get(dport);
    if (nullptr == session) {
        icmp_reject(ICMP_V4_DST_UNREACHABLE, ICMP_V4_PORT_UNREACHABLE, packet);
        goto error;
    }

    (*session)->rxQueue.push(packet);
    goto exit;

error:
    ret = -1;
    nbuf_free(packet);

exit:
    return ret;
}

NetworkBuffer *udp_alloc_nbuf(IpAddress destinationIp, uint16_t destinationPort, uint16_t size)
{
    NetworkBuffer* nbuf = ip_alloc_nbuf(destinationIp, 64, IPPROTO_UDP, sizeof(udp_t) + size);
    if (!nbuf) {
        return nullptr;
    }

    udp_t* udp = udp_hdr(nbuf);
    udp->dport = htons(destinationPort);
    udp->sport = 0;
    udp->length = htons((uint16_t) nbuf_size(nbuf));
    udp->csum = 0;

    return nbuf;
}

static int udp_proto_init(void)
{
    socket_register_triple({AF_INET, SOCK_DGRAM, IPPROTO_UDP}, []() {
        return std::unique_ptr<SocketFileDescriptor>(new UdpFileDescriptor());
    });

    return 0;
}