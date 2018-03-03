#include <unistd.h>
#include <platform/drivers.h>
#include <vfs/VirtualFileSystem.h>
#include "ConsoleFileDescriptor.h"
#include "DevFileSystem.h"

static int dev_console_init(void)
{
    auto fd = VirtualFileSystem::instance().open("/dev", O_WRONLY);
    if (!fd) {
        return 1;
    }

    DevFileSystem::IoctlRegisterDevice cmdbuf;
    cmdbuf.deviceName = "console";
    cmdbuf.fdFactory = []() {
        return std::unique_ptr<FileDescriptor>(new ConsoleFileDescriptor());
    };

    return fd->ioctl((int)DevFileSystem::IoctlCommands::RegisterDevice, cmdbuf);
}

DECLARE_DRIVER(dev_console, dev_console_init, STAGE_SECOND + 1);

int ConsoleFileDescriptor::read(void* buffer, size_t size)          { return ::read(0, buffer, size); }
int ConsoleFileDescriptor::write(const void* buffer, size_t size)   { return ::write(0, buffer, size); }
int ConsoleFileDescriptor::seek(int where, int whence)              { return -1; }
void ConsoleFileDescriptor::close(void)                             { }

int ConsoleFileDescriptor::stat(struct stat *stat) {
    return -1;
}
