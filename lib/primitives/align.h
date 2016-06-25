#ifndef __ALIGN_H__
#define __ALIGN_H__

#define rounddown(x, y) (((x) / (y)) * (y))
#define roundup(x, y)   ((((x) + ((y) - 1)) / (y)) * (y))
#define powerof2(x)     ((((x) - 1) & (x)) == 0)

/* Aligns the address to given size (must be power of 2) */
#define align(addr, size) ({                \
    intptr_t _addr = (intptr_t)(addr);      \
    intptr_t _size = (intptr_t)(size);      \
    _addr = (_addr + (_size - 1)) & -_size; \
    (__typeof__(addr))_addr; })

#define is_aligned(addr, size) ({           \
    intptr_t _addr = (intptr_t)(addr);      \
    intptr_t _size = (intptr_t)(size);      \
    !(_addr & (_size - 1)); })

#endif // __ALIGN_H__
