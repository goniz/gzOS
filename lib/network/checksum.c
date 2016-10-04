#include "checksum.h"

static inline unsigned short from32to16(unsigned long x)
{
    /* add up 16-bit and 16-bit for 16+c bit */
    x = (x & 0xffff) + (x >> 16);
    /* add up carry.. */
    x = (x & 0xffff) + (x >> 16);
    return x;
}

static unsigned int do_csum(const unsigned char *buff, int len)
{
    int odd, count;
    unsigned long result = 0;

    if (len <= 0)
        goto out;
    odd = 1 & (unsigned long) buff;
    if (odd) {
#ifdef __LITTLE_ENDIAN
        result = *buff;
#else
        result += (*buff << 8);
#endif
        len--;
        buff++;
    }
    count = len >> 1;		/* nr of 16-bit words.. */
    if (count) {
        if (2 & (unsigned long) buff) {
            result += *(unsigned short *) buff;
            count--;
            len -= 2;
            buff += 2;
        }
        count >>= 1;		/* nr of 32-bit words.. */
        if (count) {
            unsigned long carry = 0;
            do {
                unsigned long w = *(unsigned int *) buff;
                count--;
                buff += 4;
                result += carry;
                result += w;
                carry = (w > result);
            } while (count);
            result += carry;
            result = (result & 0xffff) + (result >> 16);
        }
        if (len & 2) {
            result += *(unsigned short *) buff;
            buff += 2;
        }
    }
    if (len & 1)
#ifdef __LITTLE_ENDIAN
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
    unsigned int sum = (unsigned int)wsum;
    unsigned int result = do_csum(buff, len);

    /* add in old sum, and carry.. */
    result += sum;
    if (sum > result)
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
