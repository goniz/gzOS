#include <cstddef>
#include <lib/primitives/interrupts_mutex.h>
#include "ReaddirFileDescriptor.h"

int ReaddirFileDescriptor::read(void *buffer, size_t size) {
    InterruptsMutex mutex(true);
    if (size < sizeof(DirEntry)) {
        return -1;
    }

    struct DirEntry* dirEntry = (struct DirEntry*)buffer;
    if (this->getNextEntry(*dirEntry)) {
        return sizeof(DirEntry);
    }

    return 0;
}

int ReaddirFileDescriptor::write(const void *buffer, size_t size) {
    return -1;
}

int ReaddirFileDescriptor::seek(int where, int whence) {
    // TODO: implement
    return -1;
}

void ReaddirFileDescriptor::close(void) {

}
