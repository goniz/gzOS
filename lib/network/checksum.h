#ifndef GZOS_CHECKSUM_H
#define GZOS_CHECKSUM_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

uint32_t csum_partial(const void *buff, int len, uint32_t wsum);
uint16_t csum_partial_done(uint32_t csum);
uint16_t ip_compute_csum(const void *buff, int len);
uint16_t ip_fast_csum(const void *iph, unsigned int ihl);

#ifdef __cplusplus
}
#endif
#endif //GZOS_CHECKSUM_H
