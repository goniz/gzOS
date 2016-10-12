#include <platform/kprintf.h>
#include <cassert>
#include <cstring>
#include "ElfLoader.h"
#include "elf.h"

#define ELF32_DEBUG

ElfLoader::ElfLoader(const void *buffer, size_t size)
        : _buffer(buffer),
          _size(size)
{
    assert(NULL != _buffer);
    assert(0 != _size);
}

bool ElfLoader::sanityCheck(void) const
{
    const Elf32_Ehdr* ehdr = (Elf32_Ehdr *) _buffer;

#ifdef ELF32_DEBUG
    kprintf("\n\rFile header:");
    kprintf("\n\rmagic[]=%c%c%c%c class=%u data=%u\n",
            ehdr->e_ident[EI_MAG0],
            ehdr->e_ident[EI_MAG1],
            ehdr->e_ident[EI_MAG2],
            ehdr->e_ident[EI_MAG3],
            ehdr->e_ident[EI_CLASS],
            ehdr->e_ident[EI_DATA]
    );
#endif

    // check magic
    if (0 != memcmp(ehdr->e_ident, ELFMAG, SELFMAG)) {
        return false;
    }

    // check 32bit
    if (ehdr->e_ident[EI_CLASS] != ELFCLASS32)
        return false;

    // check big endian
    if (ehdr->e_ident[EI_DATA] != ELFDATA2MSB) {
        return false;
    }

    // check MIPS arch
    if (ehdr->e_machine != EM_MIPS) {
        return false;
    }

    return true;
}
