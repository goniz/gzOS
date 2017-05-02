//
// Created by gz on 7/16/16.
//

#include <platform/drivers.h>
#include <cstddef>
#include <cstdio>

extern struct driver_entry drivers_table[];
extern uint8_t __drivers_start;
extern uint8_t __drivers_end;

extern "C"
int drivers_init(void)
{
    const size_t n_drivers = ((&__drivers_end - &__drivers_start) / sizeof(struct driver_entry));
    int ret = 0;
    int count = 0;

    for (size_t i = 0; i < n_drivers; i++) {
        struct driver_entry* current = &drivers_table[i];

        try {
            ret = current->init();
        } catch (...) {
            ret = -1;
        }

        if (0 == ret) {
            count++;
            printf("[driver] loaded driver %s.\n", current->name);
        } else {
            printf("[driver] failed to load driver %s. (%d)\n", current->name, ret);
        }

    }

    return count;
}