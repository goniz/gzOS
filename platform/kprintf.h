//
// Created by gz on 6/11/16.
//

#ifndef GZOS_KPRINTF_H
#define GZOS_KPRINTF_H

#include <stdarg.h>

void vkprintf(const char* fmt, va_list arg);
void kprintf(const char* fmt, ...);

#endif //GZOS_KPRINTF_H
