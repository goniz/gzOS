#include <unistd.h>
#include "ConsoleFileDescriptor.h"

int ConsoleFileDescriptor::read(void* buffer, size_t size) {
    return ::read(0, buffer, size);
}

int ConsoleFileDescriptor::write(const void* buffer, size_t size) {
    return ::write(0, buffer, size);
}

int ConsoleFileDescriptor::seek(int where, int whence) {
    return -1;
}

void ConsoleFileDescriptor::close(void) {

}
