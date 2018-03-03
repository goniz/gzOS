#include <lib/syscall/syscall.h>
#include <sys/time.h>
#include <vector>
#include <proc/Scheduler.h>
#include <platform/cpu.h>
#include <lib/primitives/time.h>
#include <platform/clock.h>

struct fd_action_state {
    bool requested = false;
    bool current = false;
};

struct monitored_fd {
    monitored_fd(int fd)
        : fd(fd)
    {

    }

    int fd;
    fd_action_state read;
    fd_action_state write;
    fd_action_state except;
};

static std::vector<monitored_fd> monitored_fds_from_fd_sets(int maxFd,
                                                            const fd_set* read_fds,
                                                            const fd_set* write_fds,
                                                            const fd_set* except_fds)
{
    std::vector<monitored_fd> fds;

    for (int i = 0; i < maxFd; i++) {
        monitored_fd fd(i);


        fd.read.requested = read_fds && FD_ISSET(i, read_fds);
        fd.write.requested = write_fds && FD_ISSET(i, write_fds);
        fd.except.requested = except_fds && FD_ISSET(i, except_fds);

        if (fd.read.requested || fd.write.requested || fd.except.requested) {
            fds.push_back(fd);
        }
    }

    return std::move(fds);
}

static void fd_sets_from_monitored_fds(const std::vector<monitored_fd>& fds,
                                       fd_set* read_fds,
                                       fd_set* write_fds,
                                       fd_set* except_fds)
{
    for (const auto& fd : fds) {
        if (read_fds && fd.read.requested && fd.read.current) {
            FD_SET(fd.fd, read_fds);
        }

        if (write_fds && fd.write.requested && fd.write.current) {
            FD_SET(fd.fd, write_fds);
        }

        if (except_fds && fd.except.requested && fd.except.current) {
            FD_SET(fd.fd, except_fds);
        }
    }
}

static void zero_fd_set(fd_set* set) {
    if (set) {
        FD_ZERO(set);
    }
}

static int do_select_loop(FileDescriptorCollection& fdsCollection,
                          std::vector<monitored_fd>& fds)
{
    int fds_ready = 0;

    for (auto& watched_fd : fds) {
        InterruptsMutex guard(true);
        FileDescriptor* filedesc = fdsCollection.get_filedescriptor(watched_fd.fd);

        // if the fd is invalid, or closed
        // then we want to give it back to the user so he can take action
        // same with getting an error on fd->poll()
        if (!filedesc ||
            (0 != filedesc->poll(&watched_fd.read.current, &watched_fd.write.current)))
        {
            watched_fd.read.current = watched_fd.read.requested;
            watched_fd.write.current = watched_fd.write.requested;
            watched_fd.except.current = watched_fd.except.requested;
            fds_ready++;
            continue;
        }

        // now.. if poll() got us to fulfill some request, go for it.
        if ((watched_fd.read.requested && watched_fd.read.current) ||
            (watched_fd.write.requested && watched_fd.write.current) ||
            (watched_fd.except.requested && watched_fd.except.current))
        {
            fds_ready++;
            continue;
        }
    }

    return fds_ready;
}

DEFINE_SYSCALL(SELECT, select, SYS_IRQ_ENABLED)
{
    SYSCALL_ARG(int, nfds);
    SYSCALL_ARG(fd_set*, read_fds);
    SYSCALL_ARG(fd_set*, write_fds);
    SYSCALL_ARG(fd_set*, except_fds);
    SYSCALL_ARG(struct timeval*, timeout);

#define BREAK_IF_TIMED_OUT() \
    if (timeout && (clock_get_ms() > target_ms)) { break; }

    const time_t timeout_ms = time_timeval_to_ms(timeout);
    const uint64_t start_ms = clock_get_ms();
    const uint64_t target_ms = start_ms + timeout_ms;

    auto* current = Scheduler::instance().CurrentThread();
    if (!current) {
        return -1;
    }

    auto& fdsCollection = current->proc().fileDescriptorCollection();
    std::vector<monitored_fd> fds = monitored_fds_from_fd_sets(nfds, read_fds, write_fds, except_fds);

    // ZERO the IN/OUT params
    zero_fd_set(read_fds);
    zero_fd_set(write_fds);
    zero_fd_set(except_fds);

    int fds_ready = 0;
    do
    {
        BREAK_IF_TIMED_OUT();

        fds_ready = do_select_loop(fdsCollection, fds);

        BREAK_IF_TIMED_OUT();

        // let others play as well..
        syscall(SYS_NR_YIELD);

        BREAK_IF_TIMED_OUT();

//         relax cpu
        platform_cpu_wait();

        BREAK_IF_TIMED_OUT();

    } while (0 >= fds_ready);

    fd_sets_from_monitored_fds(fds, read_fds, write_fds, except_fds);

    return fds_ready;
}
