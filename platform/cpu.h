//
// Created by gz on 6/24/16.
//

#ifndef GZOS_CPU_H
#define GZOS_CPU_H

#ifdef __cplusplus
extern "C" {
#endif

int platform_read_cpu_config(void);
void platform_dump_additional_cpu_info(void);

#ifdef __cplusplus
}
#endif
#endif //GZOS_CPU_H
