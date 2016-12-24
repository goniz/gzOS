#ifndef USERLAND_MOUNT_H
#define USERLAND_MOUNT_H

#ifdef __cplusplus
extern "C" {
#endif

int mount(const char* fstype, const char* source, const char* destination);

#ifdef __cplusplus
}
#endif //extern "C"
#endif //USERLAND_MOUNT_H
