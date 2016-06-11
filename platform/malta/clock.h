#ifndef __CLOCK_H__
#define __CLOCK_H__

#include <stdint.h>

#define CPU_CLOCK 200
#define TICKS_PER_MS (1000 * CPU_CLOCK / 2)

#ifdef __cplusplus
extern "C" {
#endif

/* Initializes and enables core timer interrupts. */
void clock_init();

/* Returns the number of ms passed since timer started running. */
uint32_t clock_get_ms();

/* Processes core timer interrupts. */
struct user_regs* clock_tick_isr(struct user_regs* regs);

#ifdef __cplusplus
}
#endif

#endif // __CLOCK_H__
