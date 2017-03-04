#include <lib/syscall/syscall.h>
#include <lib/network/nbuf.h>
#include <lib/primitives/basic_queue.h>
#include <lib/kernel/sched/scheduler.h>
#include <lib/primitives/Timer.h>
#include <lib/network/checksum.h>
#include <lib/mm/physmem.h>
#include <lib/network/tcp/tcp.h>
#include <lib/network/tcp/tcp_out_segment.h>
#include <algorithm>
#include <platform/drivers.h>

MALLOC_DEFINE(_metadata_pool, "TcpOut segments metadata pool");
static basic_queue<std::shared_ptr<TcpOutSegment>> _tcp_out_segments(150);
static std::vector<std::shared_ptr<TcpOutSegment>> _tcp_pending;

static void deleter(TcpOutSegment* ptr) {
    InterruptsMutex guard(true);

    ptr->valid = false;

    if (nbuf_is_valid(ptr->packet)) {
        nbuf_free(ptr->packet);
        ptr->packet = nullptr;
    }

    if (ptr->retransmissionTimer) {
        ptr->retransmissionTimer->stop();
    }

    kfree(_metadata_pool, ptr);
}

std::shared_ptr<TcpOutSegment> tcp_out_queue_packet(NetworkBuffer* nbuf, TcpOutSegmentErrorFunc errorCallback, void* arg)
{
    if (!nbuf_is_valid(nbuf)) {
        return nullptr;
    }

    InterruptsMutex guard(true);
    auto* out_segment = (TcpOutSegment*) kmalloc(_metadata_pool, sizeof(TcpOutSegment), M_NOWAIT | M_ZERO);
    if (nullptr == out_segment) {
        return nullptr;
    }
    guard.unlock();

    auto segment_shared = std::shared_ptr<TcpOutSegment>(out_segment, deleter);
    if (nullptr == segment_shared) {
        deleter(out_segment);
        return nullptr;
    }

    out_segment->valid = true;
    out_segment->packet = nbuf;
    out_segment->transmissions = 0;
    out_segment->threshold = 3;         // TODO: add as param
    out_segment->intervalInMs = 200;    // TODO: add as param
    out_segment->callback = errorCallback;
    out_segment->callback_arg = arg;
    out_segment->retransmissionTimer = nullptr;

    if (!_tcp_out_segments.push(segment_shared, false)) {
        return nullptr;
    }

    return segment_shared;
}

auto segment_retransmission = [](std::shared_ptr<TcpOutSegment>* segment) -> bool {
    _tcp_out_segments.push(*segment);
    return false;
};

__attribute__((noreturn))
int tcp_out_main(void* argument) {
    // set up the metadata malloc pool with 1 page of memory
    vm_page_t* page = pm_alloc(1);
    assert(page != NULL);

    kmalloc_add_arena(_metadata_pool, (void*)page->vaddr, PAGESIZE);

    syscall(SYS_NR_SET_THREAD_RESPONSIVE, 1);
    while (1) {

        std::remove_if(_tcp_pending.begin(), _tcp_pending.end(), [](const auto& item) -> bool {
            return nullptr == item || false == item->valid;
        });

        std::shared_ptr<TcpOutSegment> out_segment;
        if (!_tcp_out_segments.pop(out_segment, true)) {
            syscall(SYS_NR_YIELD);
            continue;
        }

        if (!out_segment || !out_segment->valid) {
            continue;
        }

        if (!nbuf_is_valid(out_segment->packet)) {
            goto discard_segment;
        }

        {
            tcp_t* tcp = tcp_hdr(out_segment->packet);
            if (0 == tcp->csum) {
                tcp->csum = tcp_checksum(out_segment->packet);
            }

#define BOOL(x) ((x) ? "true" : "false")
#define PBOOL(x) kprintf("[tcp] " SX(x) " = %s\n", BOOL((x)))

            bool is_ack = tcp->flags & TCP_FLAGS_ACK;
            bool is_syn = tcp->flags & TCP_FLAGS_SYN;
            bool is_syn_ack = is_ack && is_syn;
            bool is_rst = tcp->flags & TCP_FLAGS_RESET;
            bool is_fin = tcp->flags & TCP_FLAGS_FIN;

            PBOOL(is_ack);
            PBOOL(is_syn);
            PBOOL(is_syn_ack);
            PBOOL(is_rst);
            PBOOL(is_fin);
        }

        kprintf("ip_output(%d, %p)\n", nbuf_size(out_segment->packet), out_segment->packet);

        if (0 != ip_output(nbuf_clone(out_segment->packet))) {
            goto discard_segment;
        }

        if (++out_segment->transmissions > out_segment->threshold) {
            goto discard_segment;
        }

        _tcp_pending.push_back(out_segment);

        out_segment->retransmissionTimer = createTimer<std::shared_ptr<TcpOutSegment>>(out_segment->intervalInMs, segment_retransmission, &_tcp_pending.back());
        out_segment->retransmissionTimer->start();
        continue;

discard_segment:
        kprintf("discard_segment(%p)\n", out_segment.get());
        out_segment->valid = false;

        if (out_segment->callback) {
            out_segment->callback(out_segment, out_segment->callback_arg);
        }

        if (out_segment->retransmissionTimer) {
            out_segment->retransmissionTimer->stop();
        }

        continue;
    }
}