#ifndef USERLAND_THREAD_H
#define USERLAND_THREAD_H
#ifdef __cplusplus
extern "C" {
#endif

pid_t thread_create(const char* name, int (*thread_func)(void* argument), void* argument);

#ifdef __cplusplus
}
#endif //extern "C"
#endif //USERLAND_THREAD_H
