//
// Created by gz on 7/15/16.
//
#include <platform/malta/cacheops.h>
#include <stdint.h>
#include <platform/cpu.h>

#define cache_loop(start, end, lsize, op) do {          \
    const void *addr = (const void *)(start & ~(lsize - 1));    \
    const void *aend = (const void *)((end - 1) & ~(lsize - 1));    \
    for (; addr <= aend; addr += lsize) {               \
        mips_cache(op, addr);         \
    }                               \
} while (0)

void flush_cache(uintptr_t start_addr, uintptr_t size)
{
    int ilsize = platform_cpu_icacheline_size();
    int dlsize = platform_cpu_dcacheline_size();

    /* aend will be miscalculated when size is zero, so we return here */
    if (size == 0)
        return;

    /* flush D-cache */
    cache_loop(start_addr, start_addr + size, dlsize, HIT_WRITEBACK_INV_D);

    /* flush I-cache */
    cache_loop(start_addr, start_addr + size, ilsize, HIT_INVALIDATE_I);
}

void flush_dcache_range(uintptr_t start_addr, uintptr_t stop)
{
    int lsize = platform_cpu_dcacheline_size();

    /* end will be miscalculated when size is zero, so we return here */
    if (start_addr == stop)
        return;

    cache_loop(start_addr, stop, lsize, HIT_WRITEBACK_INV_D);
}

void invalidate_dcache_range(uintptr_t start_addr, uintptr_t stop)
{
    int lsize = platform_cpu_dcacheline_size();

    /* aend will be miscalculated when size is zero, so we return here */
    if (start_addr == stop)
        return;

    cache_loop(start_addr, stop, lsize, HIT_INVALIDATE_D);
}