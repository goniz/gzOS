
#include <platform/clock.h>
#include <lib/syscall/syscall.h>
#include "scheduler.h"
#include "VirtualFileSystem.h"

extern "C" {

void scheduler_run_main(init_main_t init_main, int argc, const char** argv)
{
    std::vector<const char*> arguments(argv, argv + argc);
    scheduler()->createPreemptiveProcess("init", init_main, std::move(arguments), 8096);
    clock_set_handler(ProcessScheduler::onTickTimer, scheduler());
}

int scheduler_signal_process(pid_t pid, int signal) {
    InterruptsMutex mutex;
    mutex.lock();
    return scheduler()->signalProc(pid, signal);
}

int scheduler_set_timeout(int timeout_ms, timeout_callback_t callback, void* arg) {
    return scheduler()->setTimeout(timeout_ms, (ProcessScheduler::TimeoutCallbackFunc) callback, arg);
}

void scheduler_sleep(int timeout_ms) {
    scheduler()->sleep(PID_CURRENT, timeout_ms);
}

void scheduler_suspend(void) {
    scheduler()->suspend(PID_CURRENT);
}

void scheduler_resume(pid_t pid) {
    scheduler()->resume(pid);
}

pid_t scheduler_current_pid(void)
{
    return scheduler()->getCurrentPid();
}

FileDescriptor* vfs_num_to_fd(int fdnum)
{
    const auto proc = scheduler()->getCurrentProcess();
    if (nullptr == proc) {
        return nullptr;
    }

    FileDescriptorCollection& fdc = proc->fileDescriptorCollection();
    return fdc.get_filedescriptor(fdnum);
}

int vfs_open(const char* path, int flags) {
    auto fd = vfs()->open(path, flags);
    if (nullptr == fd) {
        return -1;
    }

    const auto proc = scheduler()->getCurrentProcess();
    if (nullptr == proc) {
        return -1;
    }

    FileDescriptorCollection& fdc = proc->fileDescriptorCollection();
    return fdc.push_filedescriptor(std::move(fd));
}

int vfs_close(int fd)
{
    const auto proc = scheduler()->getCurrentProcess();
    if (nullptr == proc) {
        return -1;
    }

    FileDescriptorCollection& fdc = proc->fileDescriptorCollection();
    fdc.remove_filedescriptor(fd, true);
    return 0;
}

int vfs_read(int fd, void* buf, size_t size)
{
    auto fileDes = vfs_num_to_fd(fd);
    if (nullptr == fileDes) {
        return -1;
    }

    return fileDes->read(buf, size);
}

int vfs_write(int fd, const void* buf, size_t size)
{
    auto fileDes = vfs_num_to_fd(fd);
    if (nullptr == fileDes) {
        return -1;
    }

    return fileDes->write(buf, size);
}

int vfs_seek(int fd, off_t offset, int whence)
{
    auto fileDes = vfs_num_to_fd(fd);
    if (nullptr == fileDes) {
        return -1;
    }

    return fileDes->seek(offset, whence);
}

}

int scheduler_syscall_handler(struct user_regs **regs, const struct kernel_syscall* syscall, va_list args) {
    return scheduler()->syscall_entry_point(regs, syscall, args);
}

// extern "C"