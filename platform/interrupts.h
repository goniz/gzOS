#ifndef __PLAT_INTERRUTPS_H__
#define __PLAT_INTERRUTPS_H__

#ifdef __cplusplus
extern "C" {
#endif

struct user_regs;

void interrupts_init(void);
void interrupts_enable_all(void);
unsigned int interrupts_disable(void);
void interrupts_enable(unsigned int mask);

#ifdef __cplusplus
}
#endif
#endif //__PLAT_INTERRUTPS_H__
