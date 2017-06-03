#include <algorithm>
#include <sys/param.h>
#include <lib/primitives/InterruptsMutex.h>
#include <lib/primitives/basic_queue.h>
#include "lib/network/socket.h"

class RawIpFileDescriptor;
extern std::vector<RawIpFileDescriptor*> gRawSockets;

class RawIpFileDescriptor : public SocketFileDescriptor
{
public:
    RawIpFileDescriptor() : rxQueue(RX_QUEUE_SIZE) {
        InterruptsMutex mutex;
        mutex.lock();
        gRawSockets.push_back(this);
        mutex.unlock();
    }

    virtual ~RawIpFileDescriptor(void) {
        InterruptsMutex mutex;
        mutex.lock();
        const auto iter = std::find(gRawSockets.begin(), gRawSockets.end(), this);
        if (gRawSockets.end() != iter) {
            gRawSockets.erase(iter);
        }
        mutex.unlock();
    }

    virtual int read(void *buffer, size_t size) override {
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

        mutex.unlock();
    }

private:
    friend int ip_input(NetworkBuffer *incomingPacket);

    basic_queue<NetworkBuffer*> rxQueue;
};