#include <unistd.h>
#include <platform/drivers.h>
#include "ConsoleFileDescriptor.h"
#include "VirtualFileSystem.h"
#include "DevFileSystem.h"

static int dev_console_init(void)
{
    auto* devFs = dynamic_cast<DevFileSystem*>(VirtualFileSystem::instance().getFilesystem("/dev"));
    if (!devFs) {
        return 1;
    }

    devFs->registerDevice("console", []() {
        return std::unique_ptr<FileDescriptor>(new ConsoleFileDescriptor());
    });
    return 0;
}

DECLARE_DRIVER(dev_console, dev_console_init, STAGE_SECOND + 1);

int ConsoleFileDescriptor::read(void* buffer, size_t size)          { return ::read(0, buffer, size); }
int ConsoleFileDescriptor::write(const void* buffer, size_t size)   { return ::write(0, buffer, size); }
int ConsoleFileDescriptor::seek(int where, int whence)              { return -1; }
void ConsoleFileDescriptor::close(void)                             { }
