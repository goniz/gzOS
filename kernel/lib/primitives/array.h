#ifndef GZOS_ARRAY_H
#define GZOS_ARRAY_H
#ifdef __cplusplus
extern "C" {
#endif

#define ARRAY_SIZE(arr) (sizeof((arr)) / sizeof((arr)[0]))

#ifdef __cplusplus
}
#endif //extern "C"
#endif //GZOS_ARRAY_H
