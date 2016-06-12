#include <platform/interrupts.h>
#include <platform/malta/interrupts.h>
#include <mips.h>
#include <platform/clock.h>
#include <platform/malta/clock.h>
#include <stddef.h>

/* This counter is incremented every millisecond. */
static volatile uint32_t timer_ms_count = 0;

/* this is the second stage handler to be called on every ms */
static clock_tick_handler_t timer_tick_handler = NULL;
static void* timer_tick_handler_arg = NULL;

void clock_init()
{
    unsigned int isrMask = interrupts_disable();

    mips32_set_c0(C0_COUNT, 0);
    mips32_set_c0(C0_COMPARE, TICKS_PER_MS);

    timer_ms_count = 0;
    timer_tick_handler = NULL;
    timer_tick_handler_arg = NULL;

    /* Enable core timer interrupts. */
    isrMask |= SR_IM7;

    /* It is safe now to re-enable interrupts. */
    interrupts_enable(isrMask);
}

uint32_t clock_get_ms()
{
    return timer_ms_count;
}

struct user_regs* clock_tick_isr(struct user_regs* regs)
{
    uint32_t compare = mips32_get_c0(C0_COMPARE);
    uint32_t count = mips32_get_c0(C0_COUNT);
    int32_t diff = compare - count;

    /* Should not happen. Potentially spurious interrupt. */
    if (diff > 0) {
        return regs;
    }

    /* This loop is necessary, because sometimes we may miss some ticks. */
    while (diff < TICKS_PER_MS) {
        compare += TICKS_PER_MS;
        /* Increment the ms counter. This increment is atomic, because
             entire _interrupt_handler disables nested interrupts. */
        timer_ms_count += 1;
        diff = compare - count;
    }

    if (NULL != timer_tick_handler) {
        regs = timer_tick_handler(timer_tick_handler_arg, regs);
    }

    /* Set compare register. */
    mips32_set_c0(C0_COMPARE, compare);

    return regs;
}

void clock_set_handler(clock_tick_handler_t handler, void* argument)
{
    uint32_t isrMask = interrupts_disable();
    timer_tick_handler = handler;
    timer_tick_handler_arg = argument;
    interrupts_enable(isrMask);
}
