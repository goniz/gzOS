#ifndef GZOS_WORKQUEUE_H
#define GZOS_WORKQUEUE_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

typedef void (*workqueue_func_t)(void* arg);

bool workqueue_put(workqueue_func_t work, void* arg);

#ifdef __cplusplus
}
#endif //extern "C"
#endif //GZOS_WORKQUEUE_H
