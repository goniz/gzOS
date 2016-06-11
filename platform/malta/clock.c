#include <platform/interrupts.h>
#include <platform/malta/interrupts.h>
#include <mips.h>
#include <clock.h>
#include <platform/kprintf.h>
#include <stddef.h>

/* This counter is incremented every millisecond. */
static volatile uint32_t timer_ms_count;

void clock_init()
{
    unsigned int isrMask = interrupts_disable();

    mips32_set_c0(C0_COUNT, 0);
    mips32_set_c0(C0_COMPARE, TICKS_PER_MS);

    timer_ms_count = 0;

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

    /* Set compare register. */
    mips32_set_c0(C0_COMPARE, compare);
    return regs;
}
