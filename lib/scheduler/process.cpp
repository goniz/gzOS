//
// Created by gz on 6/12/16.
//


#include <lib/scheduler/process.h>
#include <cstring>
#include <platform/process.h>

Process::Process(const char *name,
                 EntryPointFunction entryPoint, std::vector<const char*>&& arguments,
                 size_t stackSize,
                 enum Type procType, uint32_t initialQuantum)

    : _state(State::READY),
      _type(procType),
      _context(nullptr),
      _quantum(initialQuantum),
      _exitCode(0),
      _stackSize(stackSize),
      _stack(std::make_unique<uint8_t[]>(stackSize)),
      _entryPoint(entryPoint),
      _arguments(std::move(arguments))
{
    strncpy(_name, name, sizeof(_name));
    memset(_stack.get(), 0, _stackSize);

    struct process_entry_info info{Process::processMainLoop, this};
    _context = platform_initialize_process_stack(_stack.get(), _stackSize, &info);
}

__attribute__((noreturn))
void Process::processMainLoop(void* argument)
{
    Process* self = (Process*)argument;

    self->_exitCode = self->_entryPoint(self->_arguments.size(), self->_arguments.data());
    self->_state = State::TERMINATED;

    while (true);
}



