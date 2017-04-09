#include <stddef.h>
#include <stdio.h>
#include <platform/kprintf.h>
#include "hexdump.h"

void hexDump(const char* desc, const void* addr, int len)
{
    int i;
    unsigned char buff[17];
    unsigned char *pc = (unsigned char *) addr;

    // Output description if given.
    if (desc != NULL)
        kprintf("%s:\n", desc);

    if (len == 0) {
        kprintf("  ZERO LENGTH\n");
        return;
    }
    if (len < 0) {
        kprintf("  NEGATIVE LENGTH: %i\n", len);
        return;
    }

    // Process every byte in the data.
    for (i = 0; i < len; i++) {
        // Multiple of 16 means new line (with line offset).

        if ((i % 16) == 0) {
            // Just don't print ASCII for the zeroth line.
            if (i != 0)
                kprintf("  %s\n", buff);

            // Output the offset.
            kprintf("  %04x ", i);
        }

        // Now the hex code for the specific character.
        kprintf(" %02x", pc[i]);

        // And store a printable ASCII character for later.
        if ((pc[i] < 0x20) || (pc[i] > 0x7e))
            buff[i % 16] = '.';
        else
            buff[i % 16] = pc[i];
        buff[(i % 16) + 1] = '\0';
    }

    // Pad out last line if not exactly 16 characters.
    while ((i % 16) != 0) {
        kprintf("   ");
        i++;
    }

    // And print the final ASCII bit.
    kprintf("  %s\n", buff);
}