#ifndef USERLAND_VLAD_H
#define USERLAND_VLAD_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

struct vlad_handle;
typedef struct vlad_handle* vlad_handle_t;
typedef bool (*vlad_handler_func)(void* arg);

vlad_handle_t vlad_init(void);
void vlad_free(vlad_handle_t handle);

bool vlad_register_fd(vlad_handle_t handle, int fd, vlad_handler_func handler, void* arg);
void vlad_unregister_fd(vlad_handle_t handle, int fd);

bool vlad_loop_once(vlad_handle_t handle);
void vlad_loop(vlad_handle_t handle);

#ifdef __cplusplus
}
#endif //extern "C"
#endif //USERLAND_VLAD_H
