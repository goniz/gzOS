#ifndef GZOS_NBUF_H
#define GZOS_NBUF_H

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include <lib/primitives/align.h>

#define NetworkBufferControlPoolSize    (512 * 1024)        // 500KB
#define NetworkBufferDataPoolSize       (1 * 1024 * 1024)   // 1MB

#undef NBUF_DEBUG

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    void* buffer;
    size_t buffer_capacity;
    size_t buffer_size;
} PacketBuffer;

typedef struct {
    PacketBuffer buffer;
    const char* device;
    void* l2_offset;
    uint16_t l3_proto;
    void* l3_offset;
    uint16_t l4_proto;
    void* l4_offset;
} NetworkBuffer;

NetworkBuffer* nbuf_alloc(size_t size);
NetworkBuffer* nbuf_alloc_aligned(size_t size, int alignment);\
NetworkBuffer* nbuf_clone(const NetworkBuffer *nbuf);
NetworkBuffer* nbuf_clone_offset_nometadata(const NetworkBuffer* nbuf, size_t startOffset);
void nbuf_free(NetworkBuffer* nbuf);

static inline int nbuf_is_valid(const NetworkBuffer* nbuf) {
    return (NULL != nbuf) && (NULL != nbuf->buffer.buffer);
}

static inline void* nbuf_data(const NetworkBuffer* nbuf) {
    return nbuf->buffer.buffer;
}

static inline size_t nbuf_size(const NetworkBuffer* nbuf) {
    return nbuf->buffer.buffer_size;
}

static inline size_t nbuf_capacity(const NetworkBuffer* nbuf) {
    return nbuf->buffer.buffer_capacity;
}

static inline const char* nbuf_device(const NetworkBuffer* nbuf) {
    return nbuf->device;
}

static inline size_t nbuf_size_from(const NetworkBuffer* nbuf, const void* pos) {
    assert(pointer_is_in_range(pos, nbuf->buffer.buffer, nbuf->buffer.buffer_capacity));
    uintptr_t start = (uintptr_t) pos;
    uintptr_t end = (uintptr_t) nbuf->buffer.buffer + nbuf->buffer.buffer_size;
    return end - start;
}

static inline void nbuf_set_device(NetworkBuffer* nbuf, const char* deviceName) {
    nbuf->device = deviceName;
}

static inline void nbuf_set_l2(NetworkBuffer* nbuf, void* buffer) {
    assert(pointer_is_in_range(buffer, nbuf->buffer.buffer, nbuf->buffer.buffer_capacity));
    nbuf->l2_offset = buffer;
}

static inline void nbuf_set_l3(NetworkBuffer* nbuf, void* buffer, uint16_t proto) {
    assert(pointer_is_in_range(buffer, nbuf->buffer.buffer, nbuf->buffer.buffer_capacity));
    nbuf->l3_offset = buffer;
    nbuf->l3_proto = proto;
}

static inline void nbuf_set_l4(NetworkBuffer* nbuf, void* buffer, uint16_t proto) {
    assert(pointer_is_in_range(buffer, nbuf->buffer.buffer, nbuf->buffer.buffer_capacity));
    nbuf->l4_offset = buffer;
    nbuf->l4_proto = proto;
}

static inline void nbuf_set_size(NetworkBuffer* nbuf, size_t size) {
    assert(size <= nbuf->buffer.buffer_capacity);
    nbuf->buffer.buffer_size = size;
}

static inline ptrdiff_t nbuf_offset(const NetworkBuffer* nbuf, const void* buffer) {
    assert(pointer_is_in_range(buffer, nbuf->buffer.buffer, nbuf->buffer.buffer_capacity));
    return (uintptr_t)buffer - (uintptr_t)(nbuf->buffer.buffer);
}

#ifdef __cplusplus
};
#endif
#endif //GZOS_NBUF_H
