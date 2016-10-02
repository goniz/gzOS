
#include <lib/syscall/syscall.h>
#include <cstring>
#include <lib/network/socket.h>
#include "process.h"
#include "scheduler.h"
#include "VirtualFileSystem.h"

DEFINE_SYSCALL(SYS_NR_CREATE_PREEMPTIVE_PROC, create_preemptive_process)
{
    SYSCALL_ARG(const char*, name);
    SYSCALL_ARG(Process::EntryPointFunction, main);
    SYSCALL_ARG(int, argc);
    SYSCALL_ARG(const char**, argv);
    SYSCALL_ARG(size_t, stackSize);

    std::vector<const char*> arguments(argv, argv + argc);
    return scheduler()->createPreemptiveProcess(name, main, std::move(arguments), stackSize);
}

DEFINE_SYSCALL(SYS_NR_CREATE_RESPONSIVE_PROC, create_responsive_process)
{
    SYSCALL_ARG(const char*, name);
    SYSCALL_ARG(Process::EntryPointFunction, main);
    SYSCALL_ARG(int, argc);
    SYSCALL_ARG(const char**, argv);
    SYSCALL_ARG(size_t, stackSize);

    std::vector<const char*> arguments(argv, argv + argc);
    return scheduler()->createResponsiveProcess(name, main, std::move(arguments), stackSize);
}

DEFINE_SYSCALL(SYS_NR_GET_PID, get_pid)
{
    return scheduler()->getCurrentPid();
}

DEFINE_SYSCALL(SYS_NR_YIELD, yield)
{
    *regs = scheduler()->yield(*regs);
    return 0;
}

DEFINE_SYSCALL(SYS_NR_SIGNAL, signal)
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

DEFINE_SYSCALL(SYS_NR_PS, ps)
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

DEFINE_SYSCALL(SYS_NR_SOCKET, socket)
{
    SYSCALL_ARG(int, domain);
    SYSCALL_ARG(int, type);
    SYSCALL_ARG(int, proto);

    return socket(domain, type, proto);
}

DEFINE_SYSCALL(SYS_NR_BIND, bind)
{
    SYSCALL_ARG(int, fd);
    SYSCALL_ARG(const SocketAddress*, address);

    SocketFileDescriptor* sockFd = socket_num_to_fd(fd);
    if (nullptr == sockFd) {
        return -1;
    }

    return sockFd->bind(*address);
}