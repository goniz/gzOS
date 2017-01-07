
#include <lib/syscall/syscall.h>
#include <cstring>
#include <lib/network/socket.h>
#include <vfs/vfs_api.h>
#include <fcntl.h>
#include "proccess/process.h"
#include "sched/scheduler.h"
#include "signals.h"

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

    std::vector<const char*> arguments(argv, argv + argc);
    Process* newProc = Scheduler::instance().createProcess(name, elfBuffer, elfSize, std::move(arguments), 8192);
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
    SYSCALL_ARG(size_t, stackSize);

    Process* currentProc = Scheduler::instance().CurrentProcess();
    if (NULL == currentProc) {
        return -1;
    }

    Thread* newThread = Scheduler::instance().createThread(*currentProc, name, entryPoint, argument, stackSize);
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

    std::vector<const char*> arguments(argv, argv + argc);
    Process* newProc = Scheduler::instance().createProcess(filename, buffer.get(), size, std::move(arguments), 8192);
    if (!newProc) {
        return -1;
    }

    return newProc->pid();
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

    auto& instance = Scheduler::instance();
    auto proc = instance.CurrentProcess();
    assert(proc);

    if (!instance.signalPid(PID_CURRENT, SIG_KILL)) {
        return -1;
    }

    proc->terminate(exit_code);

    *regs = instance.schedule(*regs);
    return 0;
}

DEFINE_SYSCALL(IS_THREAD_RESPONSIVE, is_thread_responsive, SYS_IRQ_DISABLED)
{
    auto thread = Scheduler::instance().CurrentThread();
    assert(thread);

    return thread->isResponsive();
}

DEFINE_SYSCALL(SET_THREAD_RESPONSIVE, set_thread_responsive, SYS_IRQ_DISABLED)
{
    SYSCALL_ARG(int, isResponsive);

    auto thread = Scheduler::instance().CurrentThread();
    assert(thread);

    thread->setResponsive((bool)isResponsive);
    return 0;
}

DEFINE_SYSCALL(YIELD, yield, SYS_IRQ_DISABLED)
{
    *regs = Scheduler::instance().yield(*regs);
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

    Scheduler::instance().sleep(PID_CURRENT, ms);
    return 0;
}

DEFINE_SYSCALL(SIGNAL, signal, SYS_IRQ_DISABLED)
{
    SYSCALL_ARG(pid_t, pid);
    SYSCALL_ARG(int, signal_nr);

    if (!Scheduler::instance().signalPid(pid, signal_nr)) {
        return -1;
    }

    *regs = Scheduler::instance().schedule(*regs);
    return 0;
}

static const char* g_states[] = {
        "Ready",
        "Running",
        "Suspended",
        "Terminated"
};

static bool fill_ps_ent(const Process* proc, struct ps_ent* ent, size_t size_left)
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

    size_t buf_pos = sizeof(struct ps_ent);
    int entries = 0;
    const auto& procList = Scheduler::instance().processList();
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

DEFINE_SYSCALL(MOUNT, mount, SYS_IRQ_DISABLED)
{
    SYSCALL_ARG(const char*, fstype);
    SYSCALL_ARG(const char*, source);
    SYSCALL_ARG(const char*, destination);

    return vfs_mount(fstype, source, destination);
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