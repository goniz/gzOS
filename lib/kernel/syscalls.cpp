
#include <lib/syscall/syscall.h>
#include <cstring>
#include <lib/network/socket.h>
#include "process.h"
#include "scheduler.h"
#include "VirtualFileSystem.h"

DEFINE_SYSCALL(CREATE_PREEMPTIVE_PROC, create_preemptive_process, SYS_IRQ_DISABLED)
{
    SYSCALL_ARG(const char*, name);
    SYSCALL_ARG(Process::EntryPointFunction, main);
    SYSCALL_ARG(int, argc);
    SYSCALL_ARG(const char**, argv);
    SYSCALL_ARG(size_t, stackSize);

    std::vector<const char*> arguments(argv, argv + argc);
    return scheduler()->createPreemptiveProcess(name, main, std::move(arguments), stackSize);
}

DEFINE_SYSCALL(CREATE_RESPONSIVE_PROC, create_responsive_process, SYS_IRQ_DISABLED)
{
    SYSCALL_ARG(const char*, name);
    SYSCALL_ARG(Process::EntryPointFunction, main);
    SYSCALL_ARG(int, argc);
    SYSCALL_ARG(const char**, argv);
    SYSCALL_ARG(size_t, stackSize);

    std::vector<const char*> arguments(argv, argv + argc);
    return scheduler()->createResponsiveProcess(name, main, std::move(arguments), stackSize);
}

DEFINE_SYSCALL(GET_PID, get_pid, SYS_IRQ_DISABLED)
{
    return scheduler()->getCurrentPid();
}

DEFINE_SYSCALL(YIELD, yield, SYS_IRQ_DISABLED)
{
    *regs = scheduler()->yield(*regs);
    return 0;
}

DEFINE_SYSCALL(SCHEDULE, schedule, SYS_IRQ_DISABLED)
{
    *regs = scheduler()->schedule(*regs);
    return 0;
}

DEFINE_SYSCALL(SLEEP, sleep, SYS_IRQ_DISABLED)
{
    SYSCALL_ARG(int, ms);

    scheduler()->sleep(PID_CURRENT, ms);
    return 0;
}

DEFINE_SYSCALL(SIGNAL, signal, SYS_IRQ_DISABLED)
{
    SYSCALL_ARG(pid_t, pid);
    SYSCALL_ARG(int, signal_nr);

    if (!scheduler()->signalProc(pid, signal_nr)) {
        return -1;
    }

    *regs = scheduler()->schedule(*regs);
    return 0;
}

static const char* g_states[] = {
        "Ready",
        "Running",
        "Suspended",
        "Yielding",
        "Terminated"
};

static const char* g_types[] = {
        "Responsive",
        "Preemptive"
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
    ent->type = g_types[proc->type()];
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
    int entries = 1;
    if (!fill_ps_ent(&scheduler()->_idleProc, (struct ps_ent *)buffer, size)) {
        return 0;
    }

    const auto& procList = scheduler()->processList();
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

DEFINE_SYSCALL(EXEC, exec, SYS_IRQ_DISABLED)
{
    SYSCALL_ARG(const void*, elfBuffer);
    SYSCALL_ARG(size_t, elfSize);

    (void)elfBuffer;
    (void)elfSize;

    return -1;
}