#ifndef GZOS_PROCESSLIST_H
#define GZOS_PROCESSLIST_H

#ifdef __cplusplus

#include "Process.h"

class ProcessProvider
{
public:
    typedef std::vector<std::unique_ptr<Process>> ProcessVector;

    ProcessProvider(void);

    Process& SystemProcess(void);

    Process* createProcess(const char* name,
                           const void* buffer, size_t size,
                           std::vector<std::string>&& arguments,
                           size_t stackSize);

    Process* createKernelProcess(const char* name,
                                 std::vector<std::string>&& arguments,
                                 bool initializeFds = false);

    Thread* createKernelThread(const char *name,
                               Thread::EntryPointFunction entryPoint, void *argument,
                               size_t stackSize,
                               bool schedule = true);

    void destroyProcess(Process* proc);

    void dumpProcesses(void);

    Thread* getThreadByTid(pid_t tid) const;
    Process* getProcessByPid(pid_t pid) const;

    const ProcessVector& processList(void) const {
        return _processList;
    }

    static ProcessProvider& instance(void);

private:
    static constexpr int InitialProcSize = 20;

    ProcessVector _processList;
    Process* _systemProcess;
};


#endif //cplusplus
#endif //GZOS_PROCESSLIST_H
