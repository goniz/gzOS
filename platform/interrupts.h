#ifndef __PLAT_INTERRUTPS_H__
#define __PLAT_INTERRUTPS_H__

#ifdef __cplusplus
extern "C" {
#endif

struct user_regs;

typedef void (*irq_handler_t)(struct user_regs* regs, void* data);

#define DEFINE_HW_IRQ(num) \
    struct user_regs* mips_hw_irq ##num (struct user_regs* regs)

void interrupts_init(void);
void interrupts_enable_all(void);
unsigned int interrupts_disable(void);
void interrupts_enable(unsigned int mask);

void platform_enable_hw_irq(int irq);
void platform_disable_hw_irq(int irq);

int platform_register_irq(int irq, const char* owner, irq_handler_t handler, void* data);
void platform_unregister_irq(int irq);
int platform_enable_irq(int irq);
void platform_disable_irq(int irq);
void platform_print_irqs(void);

int platform_is_irq_context(void);

#ifdef __cplusplus
}
#endif
#endif //__PLAT_INTERRUTPS_H__
