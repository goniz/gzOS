//
// Created by gz on 6/11/16.
//

#ifndef GZOS_PROCESS_H
#define GZOS_PROCESS_H

#include <stdint.h>
#include <memory>
#include <functional>
#include <vector>

class ProcessScheduler;
class Process
{
    friend class ProcessScheduler;
    enum State { READY, RUNNING, SUSPENDED, TERMINATED };
    enum Type { Responsive, Preemptive };

public:
    using EntryPointFunction = std::function<int(int, const char**)>;

    Process(const char* name,
            EntryPointFunction entryPoint, std::vector<const char*>&& arguments,
            size_t stackSize,
            enum Type procType, uint32_t initialQuantum);

private:
    __attribute__((noreturn))
    static void processMainLoop(void* argument);

    char _name[64];
    enum State _state;
    enum Type _type;
    struct user_regs* _context;
    uint32_t _quantum;
    int _exitCode;
    size_t _stackSize;
    std::unique_ptr<uint8_t[]> _stack;
    EntryPointFunction _entryPoint;
    std::vector<const char*> _arguments;
};

#endif //GZOS_PROCESS_H
