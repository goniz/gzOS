#ifndef GZOS_LOG_H
#define GZOS_LOG_H
#ifdef __cplusplus
extern "C" {
#endif

#define LOG2(x) (((x & 0xaaaaaaaa) ? 1 : 0) + ((x & 0xcccccccc) ? 2 : 0) + \
                 ((x & 0xf0f0f0f0) ? 4 : 0) + ((x & 0xff00ff00) ? 8 : 0) + \
                 ((x & 0xffff0000) ? 16 : 0))
#define LOG2_INVALID(type) ((type)((sizeof(type)<<3)-1))

#ifdef __cplusplus
}
#endif //extern "C"
#endif //GZOS_LOG_H
