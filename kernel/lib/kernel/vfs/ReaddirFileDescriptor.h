#ifndef GZOS_READDIRFILEDESCRIPTOR_H
#define GZOS_READDIRFILEDESCRIPTOR_H

#include "FileDescriptor.h"

#ifdef __cplusplus

enum DirEntryType {
    DIRENT_REG,
    DIRENT_DIR
};

struct DirEntry {
    char name[128];
    enum DirEntryType type;
};

class ReaddirFileDescriptor : public FileDescriptor {
public:
    virtual int read(void* buffer, size_t size) override;
    virtual int write(const void* buffer, size_t size) override;
    virtual int seek(int where, int whence) override;
    virtual void close(void) override;

    int stat(struct stat *stat) override;

protected:
    virtual bool getNextEntry(struct DirEntry& dirEntry) = 0;
};

#endif //cplusplus
#endif //GZOS_READDIRFILEDESCRIPTOR_H
