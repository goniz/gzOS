#ifndef GZOS_LOAD_INITRD_H
#define GZOS_LOAD_INITRD_H
#ifdef __cplusplus
extern "C" {
#endif

void initrd_initialize(void);
void* initrd_get_address(void);
size_t initrd_get_size(void);

#ifdef __cplusplus
}
#endif //extern "C"
#endif //GZOS_LOAD_INITRD_H
