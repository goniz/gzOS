#ifndef GZOS_FILESYSTEM_H
#define GZOS_FILESYSTEM_H

#include <memory>
#include "FileDescriptor.h"

#ifdef __cplusplus

class FileSystem {
public:
    virtual std::unique_ptr<FileDescriptor> open(const char* path, int flags) = 0;
    virtual std::unique_ptr<FileDescriptor> readdir(const char* path) = 0;
};

#endif //extern "C"
#endif //GZOS_FILESYSTEM_H
