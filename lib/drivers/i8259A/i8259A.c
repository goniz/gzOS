#include <lib/drivers/i8259A/i8259A.h>
#include <platform/cpu.h>
#include <stddef.h>
#include <string.h>
#include <platform/drivers.h>
#include <platform/kprintf.h>
#include <platform/clock.h>
#include <platform/panic.h>
#include <platform/malta/malta.h>
#include <platform/malta/gt64120_full.h>
#include <stdio.h>
#include <platform/malta/mips.h>

static volatile struct PICRegs* _pic_master;
static volatile struct PICRegs* _pic_slave;

static unsigned int _cached_irq_mask = 0xfffb;
#define cached_master_mask  (_cached_irq_mask & 0xff)
#define cached_slave_mask   (((_cached_irq_mask & 0xff00) >> 8) & 0xff)

static struct PICInterruptHandler _irq_handlers[PIC_MAX_IRQ_NUMBER];

__unused
static void pic_ack_irq(uint8_t irq) {
    irq -= I8259A_IRQ_BASE;

    if (irq & 8) {
        /* DUMMY - (do we need this?) */
        (void) _pic_slave->imr;
        _pic_slave->imr = (uint8_t) cached_slave_mask;

        /* 'Specific EOI' to slave */
        _pic_slave->cmd = (uint8_t) (0x60 + (irq & 7));
        /* 'Specific EOI' to master-IRQ2 */
        _pic_master->cmd = (0x60 + ICW1_SINGLE);
    } else {
        /* DUMMY - (do we need this?) */
        (void) _pic_master->imr;
        _pic_master->imr = (uint8_t) cached_master_mask;

        /* 'Specific EOI to master */
        _pic_master->cmd = (uint8_t) (0x60 + irq);
    }
}

/* Returns the combined value of the cascaded PICs in-service register */
static uint16_t pic_get_isr(void);

/* Returns the combined value of the cascaded PICs irq request register */
static uint16_t pic_get_irr(void);

/*
 * Careful! The 8259A is a fragile beast, it pretty
 * much _has_ to be done exactly like this (mask it
 * first, _then_ send the EOI, and the order of EOI
 * to the two 8259s is important!
 */
static int spurious_irq_mask;
__unused
static void mask_and_ack_8259A(unsigned int irq)
{
    unsigned int irqmask = (unsigned int) (1 << irq);
    unsigned int temp_irqmask = _cached_irq_mask;
    /*
     * Lightweight spurious IRQ detection. We do not want
     * to overdo spurious IRQ handling - it's usually a sign
     * of hardware problems, so we only do the checks we can
     * do without slowing down good hardware unnecessarily.
     *
     * Note that IRQ7 and IRQ15 (the two spurious IRQs
     * usually resulting from the 8259A-1|2 PICs) occur
     * even if the IRQ is masked in the 8259A. Thus we
     * can check spurious 8259A IRQs without doing the
     * quite slow i8259A_irq_real() call for every IRQ.
     * This does not cover 100% of spurious interrupts,
     * but should be enough to warn the user that there
     * is something bad going on ...
     */
    if (temp_irqmask & irqmask) {
        goto spurious_8259A_irq;
    }

    temp_irqmask |= irqmask;

    handle_real_irq:
    if (irq & 8) {
        /* DUMMY - (do we need this?) */
        __unused uint8_t shit = _pic_slave->imr;

        /* mask out this IRQ's bit before EOI */
        _pic_slave->imr = (uint8_t) (temp_irqmask >> 8);

        /* 'Specific EOI' to slave */
        _pic_slave->cmd = (uint8_t) (0x60 + (irq & 7));

        /* 'Specific EOI' to master-IRQ2 */
        _pic_master->cmd = 0x60 + 2;

        /* turn it on again */
        _pic_slave->imr = (uint8_t) cached_slave_mask;
    } else {
        /* DUMMY - (do we need this?) */
        __unused uint8_t shit = _pic_master->imr;

        /* mask out this IRQ's bit before EOI */
        _pic_master->imr = (uint8_t) (temp_irqmask & 0xff);

        /* 'Specific EOI' to master */
        _pic_master->cmd = (uint8_t) (0x60 + irq);

        /* turn it on again */
        _pic_master->imr = (uint8_t) cached_master_mask;
    }

    return;

    spurious_8259A_irq:
    /*
     * this is the slow path - should happen rarely.
     */
    if ((pic_get_isr() & (1 << irq))) {
        /*
         * oops, the IRQ _is_ in service according to the
         * 8259A - not spurious, go handle it.
         */
        goto handle_real_irq;
    }

    /*
     * At this point we can be sure the IRQ is spurious,
     * lets ACK and report it. [once per IRQ]
     */
    if (!(spurious_irq_mask & irqmask)) {
        kprintf("spurious 8259A interrupt: IRQ%d.\n", irq);
        spurious_irq_mask |= irqmask;
    }
    /* irq_err_count++; */
    /*
     * Theoretically we do not have to handle this IRQ,
     * but in Linux this does not cause problems and is
     * simpler for us.
     */
    goto handle_real_irq;
}

static void cascade_irq_handler(struct user_regs* regs, void* data)
{

}

static int driver_i8259A_init(void)
{
    memset(_irq_handlers, 0, sizeof(_irq_handlers));
    _pic_master = (volatile struct PICRegs *) platform_ioport_to_virt(PIC_MASTER_BASE);
    _pic_slave = (volatile struct PICRegs *) platform_ioport_to_virt(PIC_SLAVE_BASE);

    // mask all interrupts
    _pic_master->imr = 0xff;
    _pic_slave->imr = 0xff;

    /* ICW1: select 8259A-1 init */
    _pic_master->cmd = 0x11;
    /* ICW2: 8259A-1 IR0 mapped to I8259A_IRQ_BASE + 0x00 */
    _pic_master->imr = I8259A_IRQ_BASE + 0;
    /* 8259A-1 (the master) has a slave on IR2 */
    _pic_master->imr = (1U << ICW1_SINGLE);
    /* master expects normal EOI */
    _pic_master->imr = ICW1_ICW4;

    /* ICW1: select 8259A-2 init */
    _pic_slave->cmd = 0x11;
    /* ICW2: 8259A-2 IR0 mapped to I8259A_IRQ_BASE + 0x08 */
    _pic_slave->imr = I8259A_IRQ_BASE + 8;
    /* 8259A-2 is a slave on master's IR2 */
    _pic_slave->imr = ICW1_SINGLE;
    /* (slave's support for AEOI in flat mode is to be investigated) */
    _pic_slave->imr = ICW1_ICW4;

    /* wait for 8259A to initialize */
    clock_delay_ms(1);

    // restore cached states
    _pic_master->imr = (uint8_t) cached_master_mask;
    _pic_slave->imr = (uint8_t) cached_slave_mask;

    clock_delay_ms(1);

    platform_register_irq(2, "PIC Cascaded IRQ", cascade_irq_handler, NULL);
    platform_enable_irq(2);
    platform_enable_hw_irq(2);
    return 0;
}

/* Helper func */
static uint16_t __pic_get_irq_reg(uint8_t ocw3)
{
    /* OCW3 to PIC CMD to get the register values.  PIC2 is chained, and
     * represents IRQs 8-15.  PIC1 is IRQs 0-7, with 2 being the chain */
    _pic_master->cmd = ocw3;
    _pic_slave->cmd = ocw3;

    return ((_pic_slave->cmd << 8) | (_pic_master->cmd));
}

/* Returns the combined value of the cascaded PICs irq request register */
static uint16_t pic_get_irr(void)
{
    return __pic_get_irq_reg(PIC_READ_IRR);
}

/* Returns the combined value of the cascaded PICs in-service register */
static uint16_t pic_get_isr(void)
{
    return __pic_get_irq_reg(PIC_READ_ISR);
}

__attribute__((used))
static uint16_t pic_pending(void) {
    return (uint16_t) (pic_get_irr() & ~_cached_irq_mask);
}

int platform_register_irq(int irq, const char* owner, irq_handler_t handler, void* data)
{
    if (irq >= PIC_MAX_IRQ_NUMBER) {
        return 0;
    }

    if (_irq_handlers[irq].handler) {
        return 0;
    }

    _irq_handlers[irq].handler = handler;
    _irq_handlers[irq].data = data;
    _irq_handlers[irq].owner = owner;
    return 1;
}

void platform_unregister_irq(int irq)
{
    if (irq >= PIC_MAX_IRQ_NUMBER) {
        return;
    }

    unsigned int isrMask = interrupts_disable();

    memset(&_irq_handlers[irq], 0, sizeof(_irq_handlers[irq]));

    interrupts_enable(isrMask);
}

int platform_enable_irq(int irq)
{
    if (irq >= PIC_MAX_IRQ_NUMBER) {
        return 0;
    }

    if (NULL == _irq_handlers[irq].handler) {
        return 0;
    }

    unsigned int isrMask = interrupts_disable();

    irq -= I8259A_IRQ_BASE;
    _cached_irq_mask &= ~(1 << irq);
    _cached_irq_mask &= ~(1 << 2); // cascade irq always on..
    _pic_slave->imr = (uint8_t) cached_slave_mask;
    _pic_master->imr = (uint8_t) cached_master_mask;

    interrupts_enable(isrMask);
    return 1;
}

void platform_disable_irq(int irq)
{
    if (irq >= PIC_MAX_IRQ_NUMBER) {
        return;
    }

    unsigned int isrMask = interrupts_disable();

    irq -= I8259A_IRQ_BASE;
    _cached_irq_mask |= (1 << irq);
    if (irq & 8) {
        _pic_slave->imr = (uint8_t) cached_slave_mask;
    } else {
        _pic_master->imr = (uint8_t) cached_master_mask;
    }

    interrupts_enable(isrMask);
}

void platform_print_irqs(void)
{
    int i = 0;

    for (; i < PIC_MAX_IRQ_NUMBER; i++) {
        const struct PICInterruptHandler* pos = &_irq_handlers[i];
        if (NULL == pos->handler || 0 == (_cached_irq_mask & i)) {
            continue;
        }

        printf("irq %d - %s\n", i, pos->owner);
    }
}


#define __GT_READ(ofs)	        (*(volatile uint32_t *)(MIPS_PHYS_TO_KSEG1(MALTA_CORECTRL_BASE + (ofs))))
#define __GT_WRITE(ofs, data)	do { *(volatile uint32_t*)(MIPS_PHYS_TO_KSEG1(MALTA_CORECTRL_BASE + (ofs))) = (data); } while (0)

#define GT_READ(ofs)		    le32_to_cpu(__GT_READ(ofs))
#define GT_WRITE(ofs, data)	    __GT_WRITE(ofs, cpu_to_le32(data))

DEFINE_HW_IRQ(2)
{
    __unused
    volatile uint8_t temp = 0;
    /* Get the Interrupt */
    volatile uint8_t irq = (uint8_t )(GT_READ(GT_PCI0_IACK_OFS) & 0xff);

    /*
     *  Mask interrupts
     *   + Mask all except cascade on master
     *   + Mask all on slave
     */
    _pic_master->imr = 0xfb;
    _pic_slave->imr = 0xff;

    if (irq > PIC_MAX_IRQ_NUMBER) {
        panic("irq(%d) > PIC_MAX_IRQ_NUMBER\n", irq);
    }

    const struct PICInterruptHandler* info = &_irq_handlers[irq];
    if (NULL == info->handler) {
        kprintf("No handler for irq %d (master %02x slave %02x). disabling irq.\n", irq, _pic_master->imr, _pic_slave->imr);
        platform_disable_irq(irq);
    } else {
        info->handler(regs, info->data);
    }

    /* Reset the interrupt on the 8259 either the master or the slave chip */
    if (irq & 8) {
        temp = _pic_slave->imr;

        /* Mask all */
        _pic_slave->imr = 0xff;
        _pic_slave->cmd = (uint8_t) (PIC_EOSI + (irq & 7));
        _pic_master->cmd = SLAVE_PIC_EOSI;

    } else {
        temp = _pic_master->imr;

        /* Mask all except cascade */
        _pic_master->imr = 0xfb;
        _pic_master->cmd = (uint8_t) (PIC_EOSI + irq);
    }

    /* Restore the interrupts */
    _pic_master->imr = (uint8_t) cached_master_mask;
    _pic_slave->imr = (uint8_t) cached_slave_mask;

    return regs;
}

DECLARE_DRIVER(i8259A, driver_i8259A_init, STAGE_SECOND);
