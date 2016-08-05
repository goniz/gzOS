//
// Created by gz on 7/16/16.
//

#ifndef GZOS_8259_H
#define GZOS_8259_H

#include <platform/interrupts.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Max number of IRQs */
#define PIC_MAX_IRQ_NUMBER  (8 * 2 - 1)
#define I8259A_IRQ_BASE     (0)

/* i8259A EIC registers */
struct PICRegs {
    uint8_t cmd;
    uint8_t imr;
};

#define PIC_MASTER_BASE     (0x20)
#define PIC_SLAVE_BASE      (0xA0)

#define PIC_EOI		        (0x20)
#define ICW1_ICW4	        (0x01)		/* ICW4 (not) needed */
#define ICW1_SINGLE	        (0x02)		/* Single (cascade) mode */
#define ICW1_INTERVAL4	    (0x04)		/* Call address interval 4 (8) */
#define ICW1_LEVEL	        (0x08)		/* Level triggered (edge) mode */
#define ICW1_INIT	        (0x10)		/* Initialization - required! */

#define ICW4_8086	        (0x01)		/* 8086/88 (MCS-80/85) mode */
#define ICW4_AUTO	        (0x02)		/* Auto (normal) EOI */
#define ICW4_BUF_SLAVE	    (0x08)		/* Buffered mode/slave */
#define ICW4_BUF_MASTER	    (0x0C)		/* Buffered mode/master */
#define ICW4_SFNM	        (0x10)		/* Special fully nested (not) */
#define PIC_READ_IRR        (0x0a)      /* OCW3 irq ready next CMD read */
#define PIC_READ_ISR        (0x0b)      /* OCW3 irq service next CMD read */

struct PICInterruptHandler {
    irq_handler_t handler;
    void* data;
    const char* owner;
};

#ifdef __cplusplus
};
#endif
#endif //GZOS_8259_H
