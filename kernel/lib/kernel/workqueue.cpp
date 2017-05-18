#include <platform/drivers.h>
#include <proc/ProcessProvider.h>
#include <lib/primitives/basic_queue.h>
#include "lib/kernel/workqueue.h"

struct WorkQueueItem {
    workqueue_func_t work;
    void* argument;
};

static basic_queue<WorkQueueItem> _workqueue(100);

bool workqueue_put(workqueue_func_t work, void* arg)
{
    if (!work) {
        return false;
    }

    bool result = _workqueue.push(WorkQueueItem{work, arg}, false);
    if (result) {
//        kprintf("workqueue_put: job added\n");
    } else {
        kprintf("workqueue_put: failed to add job\n");
    }

    return result;
}

_GLIBCXX_NORETURN
static int workqueue_main(void* argument) {

    kprintf("[kernel] workqueue thread active.\n");

    while (1) {
        WorkQueueItem workItem{NULL, NULL};
        bool result = _workqueue.pop(workItem, true);
        if (!result || !workItem.work) {
            continue;
        }

//        kprintf("running workqueue item %p(%p)\n", workItem.work, workItem.argument);

        try {
            workItem.work(workItem.argument);
        } catch (...) {

        }
    }
}

static int workqueue_init(void)
{
    ProcessProvider::instance().createKernelThread("workqueue", workqueue_main, NULL, PAGESIZE, true);
    return 0;
}

DECLARE_DRIVER(workqueue, workqueue_init, STAGE_SECOND);