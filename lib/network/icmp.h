#ifndef GZOS_ICMP_H
#define GZOS_ICMP_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint8_t type;
    uint8_t code;
    uint16_t csum;
    uint8_t data[];
} __attribute__((packed)) icmp_v4_t;

#ifdef __cplusplus
}
#endif
#endif //GZOS_ICMP_H
