//
// Created by gz on 7/16/16.
//

#ifndef GZOS_DRIVERS_H
#define GZOS_DRIVERS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*driver_init_t)();

struct driver_entry {
    const char* name;
    driver_init_t init;
};

#define STAGE_FIRST     0
#define STAGE_SECOND    1

#define S(x) #x
#define SX(x) S(x)
#define DECLARE_DRIVER(name, init, stage) \
            __attribute__((section(".drivers." SX(stage)),used)) \
            struct driver_entry __drv_ ##name = { SX(name), (driver_init_t) init }

int drivers_init(void);

#ifdef __cplusplus
};
#endif
#endif //GZOS_DRIVERS_H
