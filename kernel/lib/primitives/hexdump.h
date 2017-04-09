#ifndef GZOS_HEXDUMP_H
#define GZOS_HEXDUMP_H

#ifdef __cplusplus
extern "C" {
#endif

void hexDump(const char* desc, const void* addr, int len);

#ifdef __cplusplus
}
#endif
#endif //GZOS_HEXDUMP_H
