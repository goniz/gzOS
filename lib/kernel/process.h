#ifndef GZOS_PROCESS_H
#define GZOS_PROCESS_H

#ifdef __cplusplus

#include <stdint.h>
#include <memory>
#include <functional>
#include <vector>
#include <sys/types.h>
#include <atomic>
#include "FileDescriptorCollection.h"

class Scheduler;
class Thread;

class Process
{
    friend class Scheduler;
    friend class Thread;
    enum State { READY, RUNNING, SUSPENDED, TERMINATED };

public:
    using EntryPointFunction = int (*)(int, const char**);

    Process(const char* name,
            const void *buffer, size_t size,
            std::vector<const char*>&& arguments);

    ~Process(void);

    bool signal(int sig_nr);

    inline pid_t pid(void) const {
        return _pid;
    }

    const char* name(void) const;
    int exit_code(void) const;
    int state(void) const;

    FileDescriptorCollection& fileDescriptorCollection(void) {
        return _fileDescriptors;
    }

    std::vector<std::unique_ptr<Thread>>& threads(void) {
        return _threads;
    }

    uint64_t cpu_time(void) const;

private:
    Process(const char* name, std::vector<const char*>&& arguments);
    static int processMainLoop(void* argument);

    char _name[64];
    const pid_t _pid;
    enum State _state;
    int _exitCode;
    EntryPointFunction _entryPoint;
    std::vector<const char*> _arguments;
	struct platform_process_ctx* _pctx;
    std::atomic<int> _pending_signal_nr;
    FileDescriptorCollection _fileDescriptors;
    std::vector<std::unique_ptr<Thread>>   _threads;
};

#endif
#endif //GZOS_PROCESS_H
