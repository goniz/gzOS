//
// Created by gz on 6/12/16.
//

#ifndef GZOS_CLOCK_H
#define GZOS_CLOCK_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Returns the number of ms passed since timer started running. */
uint32_t clock_get_ms(void);

/* busy wait `ms` */
void clock_delay_ms(uint32_t ms);

/* Set a handler to be called on every tick. (Interrupt ctx) */
typedef struct user_regs* (*clock_tick_handler_t)(void* argument, struct user_regs* regs);
void clock_set_handler(clock_tick_handler_t handler, void* argument);

#ifdef __cplusplus
}
#endif

#endif //GZOS_CLOCK_H
