//
// Created by gz on 6/11/16.
//

#ifndef GZOS_KPRINTF_H
#define GZOS_KPRINTF_H

#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif


void vkprintf(const char *fmt, va_list arg);

void kprintf(const char *fmt, ...);

void kputs(const char* s);

#ifdef __cplusplus
}
#endif

#endif //GZOS_KPRINTF_H
