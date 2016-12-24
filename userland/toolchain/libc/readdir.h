#ifndef USERLAND_READDIR_H
#define USERLAND_READDIR_H
#ifdef __cplusplus
extern "C" {
#endif

enum DirEntryType {
    DIRENT_REG,
    DIRENT_DIR
};

struct DirEntry {
    char name[128];
    enum DirEntryType type;
};

int readdir_create(const char* path);
int readdir_read(int fd, struct DirEntry* dirent);
int readdir_close(int fd);

#ifdef __cplusplus
}
#endif //extern "C"
#endif //USERLAND_READDIR_H
