#ifndef GZOS_SBRK_H
#define GZOS_SBRK_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

void kernel_brk(void *addr);
void* kernel_sbrk(size_t size);
void* kernel_sbrk_shutdown(void);

#ifdef __cplusplus
}
#endif
#endif //GZOS_SBRK_H
