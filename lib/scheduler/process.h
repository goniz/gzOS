//
// Created by gz on 6/11/16.
//

#ifndef GZOS_PROCESS_H
#define GZOS_PROCESS_H

#include <stdint.h>
#include <memory>
#include <functional>
#include <vector>
#include <sys/types.h>

class ProcessScheduler;
class Process
{
    friend class ProcessScheduler;
    enum State { READY, RUNNING, SUSPENDED, TERMINATED };
    enum Type { Responsive, Preemptive };

public:
    using EntryPointFunction = int (*)(int, const char**);

    Process(const char* name,
            EntryPointFunction entryPoint, std::vector<const char*>&& arguments,
            size_t stackSize,
            enum Type procType, int initialQuantum);

    ~Process(void);

    inline pid_t pid(void) const {
        return _pid;
    }

private:
    __attribute__((noreturn))
    static void processMainLoop(void* argument);

    struct user_regs* _context;
    int _quantum;
    int _resetQuantum;
    enum State _state;
    char _name[64];
    pid_t _pid;
    enum Type _type;
    int _exitCode;
    EntryPointFunction _entryPoint;
    std::vector<const char*> _arguments;
	struct platform_process_ctx* _pctx;
};

#endif //GZOS_PROCESS_H
