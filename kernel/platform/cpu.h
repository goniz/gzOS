//
// Created by gz on 6/24/16.
//

#ifndef GZOS_CPU_H
#define GZOS_CPU_H

#include <stdint.h>
#include <machine/endian.h>

#ifdef __cplusplus
extern "C" {
#endif

int platform_read_cpu_config(void);
void platform_dump_additional_cpu_info(void);
int platform_cpu_dcacheline_size(void);
int platform_cpu_icacheline_size(void);

void flush_cache(uintptr_t start_addr, uintptr_t size);
void flush_dcache_range(uintptr_t start_addr, uintptr_t stop);
void invalidate_dcache_range(uintptr_t start_addr, uintptr_t stop);

uintptr_t platform_virt_to_phy(uintptr_t virt);
uintptr_t platform_phy_to_virt(uintptr_t phy);
uintptr_t platform_iomem_phy_to_virt(uintptr_t iomem);
uintptr_t platform_iomem_virt_to_phy(uintptr_t iomem);
uintptr_t platform_ioport_to_phy(uintptr_t ioport);
uintptr_t platform_ioport_to_virt(uintptr_t ioport);
uintptr_t platform_buffered_virt_to_unbuffered_virt(uintptr_t virt);

// returns the saved context on first call, returns NULL when "returning" from restoring
struct user_regs* platform_save_context(void);
void platform_restore_context(struct user_regs* ctx);
void platform_context_switch(struct user_regs* newContext, struct user_regs** oldContext);

void platform_cpu_wait(void);

#if BYTE_ORDER == BIG_ENDIAN
    #define cpu_to_be32(x)  (x)
    #define cpu_to_be16(x)  (x)
    #define cpu_to_le32(x)  (__builtin_bswap32((x)))
    #define cpu_to_le16(x)  (__builtin_bswap16((x)))
    #define be32_to_cpu(x)  (x)
    #define be16_to_cpu(x)  (x)
    #define le32_to_cpu(x)  (__builtin_bswap32((x)))
    #define le16_to_cpu(x)  (__builtin_bswap16((x)))

#elif BYTE_ORDER == LITTLE_ENDIAN
    #define cpu_to_be32(x)  (__builtin_bswap32(x))
    #define cpu_to_be16(x)  (__builtin_bswap16(x))
    #define cpu_to_le32(x)  (x)
    #define cpu_to_le16(x)  (x)
    #define be32_to_cpu(x)  (__builtin_bswap32(x))
    #define be16_to_cpu(x)  (__builtin_bswap16(x))
    #define le32_to_cpu(x)  (x)
    #define le16_to_cpu(x)  (x)

#else
    #error "What kind of system is this?"
#endif

#define htonl(x)    cpu_to_be32(x)
#define htons(x)    cpu_to_be16(x)
#define ntohl(x)    be32_to_cpu(x)
#define ntohs(x)    be16_to_cpu(x)

#define endian_set(mode, var) (var) = (mode((var)))

#define write32(base, reg, value)   (*((volatile uint32_t*)((uint32_t)(base) + (uint32_t)(reg))) = (uint32_t)(value))
#define read32(base, reg)           (*((volatile uint32_t*)((uint32_t)(base) + (uint32_t)(reg))))

#ifdef __cplusplus
}
#endif
#endif //GZOS_CPU_H
