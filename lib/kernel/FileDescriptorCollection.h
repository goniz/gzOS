#ifndef GZOS_FILEDESCRIPTORCOLLECTION_H
#define GZOS_FILEDESCRIPTORCOLLECTION_H

#include <map>
#include <memory>
#include <lib/primitives/interrupts_mutex.h>
#include "FileDescriptor.h"

#define MAX_FD      (1024)
#define INVALID_FD  (-1)

class FileDescriptorCollection {
public:
    int push_filedescriptor(std::unique_ptr<FileDescriptor> fd);
    FileDescriptor* get_filedescriptor(int fdnum);
    int remove_filedescriptor(int fdnum, bool close = true);
    void close_all(void);

private:
    int allocate_fd(void);

    std::map<int, std::unique_ptr<FileDescriptor>> _fds;
    InterruptsMutex _mutex;
};


#endif //GZOS_FILEDESCRIPTORCOLLECTION_H
