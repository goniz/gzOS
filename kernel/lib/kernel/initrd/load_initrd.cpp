#include <cmdline.h>
#include <platform/kprintf.h>
#include <cstdlib>
#include <cstdint>
#include "load_initrd.h"
#include "CpioExtractor.h"

/* sample:
 *  rd_start=0xffffffff80180000
 *  rd_size=4194304*/
void initrd_initialize(void)
{
    auto rd_start = initrd_get_address();
    auto rd_size = initrd_get_size();
    if (nullptr == rd_start || 0 == rd_size) {
        kprintf("[initrd] no initrd found. skipping\n");
        return;
    }

    kprintf("[initrd] loading initrd image @ %p (%u bytes)\n", rd_start, rd_size);

    CpioExtractor cpioExtractor(rd_start, rd_size);
    cpioExtractor.extract("/");
}

void* initrd_get_address(void) {
    auto* rd_start = cmdline_get("rd_start");
    if (!rd_start) {
        return nullptr;
    }

    uintptr_t start_addr = strtoull(rd_start, NULL, 16);
    if (0 == start_addr) {
        return nullptr;
    }

    return (void*) start_addr;
}

size_t initrd_get_size(void) {
    auto* rd_size = cmdline_get("rd_size");
    if (!rd_size) {
        return 0;
    }

    size_t size = strtoul(rd_size, NULL, 10);
    if (0 == size) {
        return 0;
    }

    return size;
}
