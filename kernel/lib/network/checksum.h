#ifndef GZOS_CHECKSUM_H
#define GZOS_CHECKSUM_H

#include <stdint.h>
#include "lib/network/nbuf.h"

#ifdef __cplusplus
extern "C" {
#endif

uint16_t checksum(const void * addr, int count, int start_sum);
uint16_t udp_checksum(const NetworkBuffer* nbuf);
uint16_t tcp_checksum(const NetworkBuffer* nbuf);

#ifdef __cplusplus
}
#endif
#endif //GZOS_CHECKSUM_H
