#include <assert.h>
#include <machine/endian.h>
#include "checksum.h"

static inline uint16_t from32to16(uint32_t x)
{
    /* add up 16-bit and 16-bit for 16+c bit */
    x = (x & 0xffff) + (x >> 16);
    /* add up carry.. */
    x = (x & 0xffff) + (x >> 16);
    return (uint16_t) x;
}

static uint32_t do_csum(const uint8_t* buff, int len)
{
    int odd, count;
    uint32_t result = 0;

    if (len <= 0)
        goto out;
    odd = (int) (1 & (uint32_t) buff);
    if (odd) {
#if BYTE_ORDER == LITTLE_ENDIAN
        result = *buff;
#else
        result += (*buff << 8);
#endif
        len--;
        buff++;
    }
    count = len >> 1;		/* nr of 16-bit words.. */
    if (count) {
        if (2 & (uint32_t) buff) {
            result += *(uint16_t*) buff;
            count--;
            len -= 2;
            buff += 2;
        }
        count >>= 1;		/* nr of 32-bit words.. */
        if (count) {
            uint32_t carry = 0;
            do {
                uint32_t w = *(uint32_t*) buff;
                count--;
                buff += 4;
                result += carry;
                result += w;
                carry = (uint32_t) (w > result);
            } while (count);
            result += carry;
            result = (result & 0xffff) + (result >> 16);
        }
        if (len & 2) {
            result += *(uint16_t *) buff;
            buff += 2;
        }
    }
    if (len & 1)
#if BYTE_ORDER == LITTLE_ENDIAN
        result += *buff;
#else
        result += (*buff << 8);
#endif
    result = from32to16(result);
    if (odd)
        result = ((result >> 8) & 0xff) | ((result & 0xff) << 8);
    out:
    return result;
}

/*
 *	This is a version of ip_compute_csum() optimized for IP headers,
 *	which always checksum on 4 octet boundaries.
 */
uint16_t ip_fast_csum(const void *iph, unsigned int ihl)
{
    return (uint16_t)~do_csum(iph, ihl*4);
}

/*
 * computes the checksum of a memory block at buff, length len,
 * and adds in "sum" (32-bit)
 *
 * returns a 32-bit number suitable for feeding into itself
 * or csum_tcpudp_magic
 *
 * this function must be called with even lengths, except
 * for the last fragment, which may be odd
 *
 * it's best to have buff aligned on a 32-bit boundary
 */
uint32_t csum_partial(const void *buff, int len, uint32_t wsum)
{
    assert(len % 2 == 0);
    uint32_t result = do_csum(buff, len);

    /* add in old sum, and carry.. */
    result += wsum;
    if (wsum > result)
        result += 1;
    return (uint32_t)result;
}

uint16_t csum_partial_done(uint32_t csum) {
    return (uint16_t) ~csum;
}

/*
 * this routine is used for miscellaneous IP-like checksums, mainly
 * in icmp.c
 */
uint16_t ip_compute_csum(const void *buff, int len)
{
    return (uint16_t)~do_csum(buff, len);
}
