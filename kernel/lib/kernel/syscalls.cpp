
#include <lib/syscall/syscall.h>
#include <cstring>
#include <lib/network/socket.h>
#include <vfs/vfs_api.h>
#include <fcntl.h>
#include <vfs/Path.h>
#include <proc/SignalProvider.h>
#include <vfs/VirtualFileSystem.h>
#include <ipc/Pipe.h>
#include "proc/Process.h"
#include "proc/Scheduler.h"
#include "signals.h"

DEFINE_SYSCALL(ENTER_SCHED, enter_sched, SYS_IRQ_DISABLED)
{
    *regs = Scheduler::instance().activate();
    return 0;
}

DEFINE_SYSCALL(CREATE_PROCESS, create_process, SYS_IRQ_DISABLED)
{
    SYSCALL_ARG(const void*, elfBuffer);
    SYSCALL_ARG(size_t, elfSize);
    SYSCALL_ARG(const char*, name);
    SYSCALL_ARG(int, argc);
    SYSCALL_ARG(const char**, argv);

    if (!elfBuffer || !elfSize || !name || !argv) {
        kprintf("%s: wrong args\n", __func__);
        return -1;
    }

    std::vector<std::string> arguments;
    arguments.reserve(argc);
    for (int i = 0; i < argc; i++) {
        arguments.emplace_back(argv[i]);
    }

    Process* newProc = ProcessProvider::instance().createProcess(name, elfBuffer, elfSize, std::move(arguments), 1*1024*1024);
    if (NULL == newProc) {
        return -1;
    }

    return newProc->pid();
}


DEFINE_SYSCALL(CREATE_THREAD, create_thread, SYS_IRQ_DISABLED)
{
    SYSCALL_ARG(const char*, name);
    SYSCALL_ARG(Thread::EntryPointFunction, entryPoint);
    SYSCALL_ARG(void*, argument);

    Process* currentProc = Scheduler::instance().CurrentProcess();
    if (NULL == currentProc) {
        return -1;
    }

    Thread* newThread = currentProc->createThread(name, entryPoint, argument, 1*1024*1024);
    if (NULL == newThread) {
        return -1;
    }

    return newThread->tid();
}

DEFINE_SYSCALL(EXEC, exec, SYS_IRQ_DISABLED)
{
    SYSCALL_ARG(const char*, filename);
    SYSCALL_ARG(int, argc);
    SYSCALL_ARG(const char**, argv);

    int fd = vfs_open(filename, O_RDONLY);
    if (fd < 0) {
        return -1;
    }

    struct stat stinfo{};
    if (0 != vfs_stat(fd, &stinfo)) {
        vfs_close(fd);
        return -1;
    }

    size_t size = (size_t) stinfo.st_size;
    auto buffer = std::make_unique<uint8_t[]>(size);
    if (-1 == vfs_read(fd, buffer.get(), size)) {
        vfs_close(fd);
        return -1;
    }

    vfs_close(fd);

    std::vector<std::string> arguments;
    arguments.reserve(argc);
    for (int i = 0; i < argc; i++) {
        arguments.emplace_back(argv[i]);
    }

    auto filenameOnly = Path(filename).filename();

    if (!ProcessProvider::instance().execProcess(*Scheduler::instance().CurrentProcess(),
                                            filenameOnly.c_str(),
                                            buffer.get(), size,
                                            std::move(arguments),
                                            1*1024*1024))
    {
        return -1;
    }

    return 0;
}

DEFINE_SYSCALL(FORK, fork, SYS_IRQ_DISABLED)
{
    Thread* calling_thread = Scheduler::instance().CurrentThread();
    if (!calling_thread) {
        return -1;
    }

    Process* child = ProcessProvider::instance().forkProcess(calling_thread->proc(), *calling_thread, *regs);
    if (!child) {
        return -1;
    }

    return child->pid();
}

DEFINE_SYSCALL(WAIT_PID, wait_pid, SYS_IRQ_ENABLED)
{
    SYSCALL_ARG(int, pid);

    return Scheduler::instance().waitForPid(pid);
}

DEFINE_SYSCALL(GET_PID, get_pid, SYS_IRQ_DISABLED)
{
    return Scheduler::instance().getCurrentPid();
}

DEFINE_SYSCALL(GET_TID, get_tid, SYS_IRQ_DISABLED)
{
    return Scheduler::instance().getCurrentTid();
}

DEFINE_SYSCALL(EXIT, exit, SYS_IRQ_DISABLED)
{
    SYSCALL_ARG(int, exit_code);
    __unused
    SYSCALL_ARG(void*, requesting_function);

    auto& instance = Scheduler::instance();
    auto* proc = instance.CurrentProcess();
    assert(NULL != proc);

//    kprintf("[sys_exit] %d: %p requested to exit(%d)\n", proc->pid(), requesting_function, exit_code);

    if (!instance.kill(proc, false)) {
        return -1;
    }

    proc->terminate(exit_code);

    *regs = instance.schedule(*regs);
    return 0;
}

DEFINE_SYSCALL(YIELD, yield, SYS_IRQ_DISABLED)
{
    auto& instance = Scheduler::instance();
    auto* thread = instance.CurrentThread();
    assert(NULL != thread);

    thread->yield();

    *regs = Scheduler::instance().schedule(*regs);
    return 0;
}

DEFINE_SYSCALL(SCHEDULE, schedule, SYS_IRQ_DISABLED)
{
    *regs = Scheduler::instance().schedule(*regs);
    return 0;
}

DEFINE_SYSCALL(SLEEP, sleep, SYS_IRQ_ENABLED)
{
    SYSCALL_ARG(int, ms);

    auto& instance = Scheduler::instance();
    auto* thread = instance.CurrentThread();
    assert(NULL != thread);

    if (thread->sleep(ms)) {
        return 0;
    }

    return -1;
}

DEFINE_SYSCALL(SIGNAL, signal, SYS_IRQ_DISABLED)
{
    SYSCALL_ARG(pid_t, pid);
    SYSCALL_ARG(int, signal_nr);
    SYSCALL_ARG(uintptr_t, value);

    if (!SignalProvider::instance().signalPid(pid, signal_nr, value)) {
        return -1;
    }

    return 0;
}

static const char* g_states[] = {
        "Ready",
        "Running",
        "Suspended",
        "Terminated"
};

static bool fill_ps_ent(Process* proc, struct ps_ent* ent, size_t size_left)
{
    if (sizeof(struct ps_ent) > size_left) {
        return false;
    }

    ent->pid = proc->pid();
    ent->exit_code = proc->exit_code();
    ent->cpu_time = (uint32_t) proc->cpu_time();
    ent->state = g_states[proc->state()];
    strncpy(ent->name, proc->name(), sizeof(ent->name));
    return true;
}

DEFINE_SYSCALL(PS, ps, SYS_IRQ_DISABLED)
{
    SYSCALL_ARG(uint8_t*, buffer);
    SYSCALL_ARG(size_t, size);

    if (nullptr == buffer || size <= 0) {
        return -1;
    }

    size_t buf_pos = 0;
    int entries = 0;
    const auto& procList = ProcessProvider::instance().processList();
    auto list_pos = procList.cbegin();
    while ((buf_pos < size) && (list_pos != procList.cend())) {
        const auto proc = list_pos->get();
        struct ps_ent* ent = (struct ps_ent*)(buffer + buf_pos);
        if (!fill_ps_ent(proc, ent, size - buf_pos)) {
            break;
        }

        entries++;
        list_pos++;
        buf_pos += sizeof(struct ps_ent);
    }

    return entries;
}

DEFINE_SYSCALL(OPEN, open, SYS_IRQ_DISABLED)
{
    SYSCALL_ARG(const char*, path);
    SYSCALL_ARG(int, flags);

    return vfs_open(path, flags);
}

DEFINE_SYSCALL(READ, read, SYS_IRQ_ENABLED)
{
    SYSCALL_ARG(int, fd);
    SYSCALL_ARG(void*, buf);
    SYSCALL_ARG(size_t, size);

    return vfs_read(fd, buf, size);
}

DEFINE_SYSCALL(WRITE, write, SYS_IRQ_DISABLED)
{
    SYSCALL_ARG(int, fd);
    SYSCALL_ARG(const void*, buf);
    SYSCALL_ARG(size_t, size);

    return vfs_write(fd, buf, size);
}

DEFINE_SYSCALL(CLOSE, close, SYS_IRQ_DISABLED)
{
    SYSCALL_ARG(int, fd);

    return vfs_close(fd);
}

DEFINE_SYSCALL(LSEEK, lseek, SYS_IRQ_DISABLED)
{
    SYSCALL_ARG(int, fd);
    SYSCALL_ARG(off_t, offset);
    SYSCALL_ARG(int, whence);

    return vfs_seek(fd, offset, whence);
}

DEFINE_SYSCALL(DUP, dup, SYS_IRQ_DISABLED)
{
    SYSCALL_ARG(int, old_fd);
    SYSCALL_ARG(int, new_fd);

    return vfs_dup(old_fd, new_fd);
}

DEFINE_SYSCALL(MOUNT, mount, SYS_IRQ_DISABLED)
{
    SYSCALL_ARG(const char*, fstype);
    SYSCALL_ARG(const char*, source);
    SYSCALL_ARG(const char*, destination);

    return vfs_mount(fstype, source, destination);
}

DEFINE_SYSCALL(MKDIR, mkdir, SYS_IRQ_DISABLED)
{
    SYSCALL_ARG(const char*, path);
    SYSCALL_ARG(mode_t, mode);

    (void)mode;

    return vfs_mkdir(path);
}

DEFINE_SYSCALL(READDIR, readdir, SYS_IRQ_DISABLED)
{
    SYSCALL_ARG(const char*, path);

    return vfs_readdir(path);
}

DEFINE_SYSCALL(BRK, brk, SYS_IRQ_DISABLED)
{
    SYSCALL_ARG(uintptr_t, pos);

    Process* currentProc = Scheduler::instance().CurrentProcess();
    if (NULL == currentProc) {
        return -1;
    }

    if (!currentProc->extendHeap(pos)) {
        return -1;
    }

    return 0;
}

DEFINE_SYSCALL(TRACEME, traceme, SYS_IRQ_DISABLED)
{
    SYSCALL_ARG(int, state);

    Process* currentProc = Scheduler::instance().CurrentProcess();
    if (NULL == currentProc) {
        return -1;
    }

    currentProc->traceme((bool) state);
    return 0;
}

DEFINE_SYSCALL(CHDIR, chdir, SYS_IRQ_DISABLED)
{
    SYSCALL_ARG(const char*, path);

    Process* currentProc = Scheduler::instance().CurrentProcess();
    if (NULL == currentProc) {
        return -1;
    }



    currentProc->changeWorkingDir(std::string(path));
    return 0;
}

DEFINE_SYSCALL(GET_CWD, getcwd, SYS_IRQ_DISABLED)
{
    SYSCALL_ARG(char*, out_path);
    SYSCALL_ARG(size_t, out_size);

    Process* currentProc = Scheduler::instance().CurrentProcess();
    if (NULL == currentProc) {
        return -1;
    }

    std::string cwd = currentProc->currentWorkingPath().string();
    strncpy(out_path, cwd.c_str(), out_size);
    return 0;
}

DEFINE_SYSCALL(PIPE, pipe, SYS_IRQ_DISABLED)
{
    SYSCALL_ARG(int*, fds);

    if (!fds) {
        return -1;
    }

    auto pipe = Pipe::create();
    if (!pipe.first || !pipe.second) {
        return -1;
    }

    fds[0] = vfs_pushfd(std::move(pipe.first));
    fds[1] = vfs_pushfd(std::move(pipe.second));

    return 0;
}

DEFINE_SYSCALL(IOCTL, ioctl, SYS_IRQ_DISABLED)
{
    SYSCALL_ARG(int, fd);
    SYSCALL_ARG(int, request);

    return vfs_vioctl(fd, request, args);
}

DEFINE_SYSCALL(DUMP_FDS, dump_fds, SYS_IRQ_DISABLED)
{
    const auto proc = Scheduler::instance().CurrentProcess();
    if (nullptr == proc) {
        return -1;
    }

    FileDescriptorCollection& fdc = proc->fileDescriptorCollection();
    fdc.dump_fds();
    return 0;
}
