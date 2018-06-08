#include <algorithm>
#include <sys/param.h>
#include <lib/primitives/InterruptsMutex.h>
#include <lib/primitives/basic_queue.h>
#include "lib/network/socket.h"

class RawIpFileDescriptor : public SocketFileDescriptor
{
public:
    RawIpFileDescriptor();
    virtual ~RawIpFileDescriptor(void);

    virtual const char* type(void) const override;
    virtual int read(void *buffer, size_t size) override;
    virtual int write(const void *buffer, size_t size) override;
    virtual int poll(bool* read_ready, bool* write_ready) override;
    virtual int seek(int where, int whence) override;
    virtual void close(void) override;

private:
    friend int ip_input(NetworkBuffer *incomingPacket);

    basic_queue<NetworkBuffer*> rxQueue;
};