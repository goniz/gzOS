//
// Created by gz on 6/24/16.
//

#ifndef GZOS_CPU_H
#define GZOS_CPU_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int platform_read_cpu_config(void);
void platform_dump_additional_cpu_info(void);

#if BYTE_ORDER == BIG_ENDIAN
    #define cpu_to_be32(x)  (x)
    #define cpu_to_be16(x)  (x)
    #define cpu_to_le32(x)  (__builtin_bswap32(x))
    #define cpu_to_le16(x)  (__builtin_bswap16(x))
    #define be32_to_cpu(x)  (x)
    #define be16_to_cpu(x)  (x)
    #define le32_to_cpu(x)  (__builtin_bswap32(x))
    #define le16_to_cpu(x)  (__builtin_bswap16(x))

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

#define write32(base, reg, value)   (*((volatile uint32_t*)((uint32_t)(base) + (uint32_t)(reg))) = (uint32_t)(value))
#define read32(base, reg)           (*((volatile uint32_t*)((uint32_t)(base) + (uint32_t)(reg))))

#ifdef __cplusplus
}
#endif
#endif //GZOS_CPU_H
