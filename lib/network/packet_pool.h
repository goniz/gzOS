#ifndef GZOS_PACKET_POOL_H
#define GZOS_PACKET_POOL_H

#include <stddef.h>
#include <stdint.h>

#define PACKET_POOL_SIZE    (1 * 1024 * 1024) // 1MB

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    void* buffer;
    size_t buffer_capacity;
    size_t buffer_size;
} PacketBuffer;

typedef struct {
    void* buffer;
    size_t size;
    PacketBuffer underlyingBuffer;
} PacketView;

PacketBuffer packet_pool_alloc(size_t size);
void packet_pool_free(PacketBuffer* ptr);
void packet_pool_free_underlying_buffer(PacketView* view);
PacketBuffer packet_pool_view_to_buffer(PacketView view);
PacketView packet_pool_view_of_buffer(PacketBuffer packetBuffer, int offset, size_t size);

#ifdef __cplusplus
};
#endif
#endif //GZOS_PACKET_POOL_H
