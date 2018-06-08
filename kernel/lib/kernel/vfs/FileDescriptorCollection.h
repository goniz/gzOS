#ifndef GZOS_FILEDESCRIPTORCOLLECTION_H
#define GZOS_FILEDESCRIPTORCOLLECTION_H

#include <map>
#include <memory>
#include <lib/primitives/InterruptsMutex.h>
#include "FileDescriptor.h"

#define MAX_FD      (1024)
#define INVALID_FD  (-1)

class FileDescriptorCollection {
public:
    FileDescriptorCollection(void) = default;
    FileDescriptorCollection(const FileDescriptorCollection& other);
    ~FileDescriptorCollection(void) = default;

    int push_filedescriptor(std::unique_ptr<FileDescriptor> fd);
    FileDescriptor* get_filedescriptor(int fdnum);
    int remove_filedescriptor(int fdnum, bool close = false);
    bool duplicate(int old_fd, int new_fd);
    void dump_fds(void) const;
    void close_all(void);

private:
    int allocate_fd(void);

    std::map<int, std::shared_ptr<FileDescriptor>> _fds;
};


#endif //GZOS_FILEDESCRIPTORCOLLECTION_H
