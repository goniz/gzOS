#include <lib/network/ip/ip_raw.h>

class RawIpFileDescriptor;
extern std::vector<RawIpFileDescriptor*> gRawSockets;

RawIpFileDescriptor::RawIpFileDescriptor(void)
    : rxQueue(RX_QUEUE_SIZE)
{
    InterruptsMutex mutex(true);

    gRawSockets.push_back(this);
}

RawIpFileDescriptor::~RawIpFileDescriptor(void)
{
    InterruptsMutex mutex(true);

    const auto iter = std::find(gRawSockets.begin(), gRawSockets.end(), this);
    if (gRawSockets.end() != iter) {
        gRawSockets.erase(iter);
    }
}

int RawIpFileDescriptor::read(void* buffer, size_t size) {
    NetworkBuffer* nbuf = NULL;
    if ((nullptr == buffer) || (!this->rxQueue.pop(nbuf, true))) {
        return -1;
    }

    iphdr_t* ip = ip_hdr(nbuf);
    size = MIN(nbuf_size_from(nbuf, ip), size);
    memcpy(buffer, ip, size);
    nbuf_free(nbuf);
    return (int) size;
}

int RawIpFileDescriptor::write(const void* buffer, size_t size) {
    return -1;
}

int RawIpFileDescriptor::seek(int where, int whence) {
    return -1;
}

void RawIpFileDescriptor::close(void) {
    NetworkBuffer* nbuf = NULL;

    while (this->rxQueue.pop(nbuf, false)) {
        nbuf_free(nbuf);
    }
}

int RawIpFileDescriptor::poll(bool* read_ready, bool* write_ready) {
    if (write_ready) {
        *write_ready = false;
    }

    if (read_ready) {
        *read_ready = !this->rxQueue.empty();
    }

    return 0;
}
