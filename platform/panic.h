//
// Created by gz on 6/11/16.
//

#ifndef GZOS_PANIC_H
#define GZOS_PANIC_H

#ifdef __cplusplus
extern "C" {
#endif

__attribute__((noreturn))
void panic(const char* fmt, ...);

#ifdef __cplusplus
}
#endif

#endif //GZOS_PANIC_H
