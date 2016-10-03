#include <lib/kernel/scheduler.h>
#include <lib/primitives/hashmap.h>
#include <lib/kernel/VirtualFileSystem.h>
#include "socket.h"

template<>
struct StringKeyConverter<socket_triple_t> {
    static void generate_key(const socket_triple_t& key, char* output_key, size_t size) {
        sprintf(output_key, "%02d:%02d:%02d", key.domain, key.type, key.protocol);
    }
};

static HashMap<socket_triple_t, SocketFileDescriptorCreator> _sockFamilies;

void socket_register_triple(socket_triple_t triple, SocketFileDescriptorCreator descriptorCreator)
{
    _sockFamilies.put(triple, std::move(descriptorCreator));
}

int socket_create(int domain, int type, int protocol)
{
    const socket_triple_t triple{(socket_family_t) domain, (socket_type_t) type, protocol};
    SocketFileDescriptorCreator* socketFileDescriptorCreator = _sockFamilies.get(triple);
    if (!socketFileDescriptorCreator) {
        return -1;
    }

    std::unique_ptr<FileDescriptor> fd = (*socketFileDescriptorCreator)();
    if (nullptr == fd) {
        return -1;
    }

    const auto proc = scheduler()->getCurrentProcess();
    if (nullptr == proc) {
        return -1;
    }

    FileDescriptorCollection& fdc = proc->fileDescriptorCollection();
    return fdc.push_filedescriptor(std::move(fd));
}

SocketFileDescriptor *socket_num_to_fd(int fdnum) {
    return dynamic_cast<SocketFileDescriptor*>(vfs_num_to_fd(fdnum));
}
