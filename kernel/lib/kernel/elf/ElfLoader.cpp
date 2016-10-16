#include <platform/kprintf.h>
#include <cassert>
#include <cstring>
#include <platform/process.h>
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

    // check for EXEC type only, no REL and things
    if (ehdr->e_type != ET_EXEC) {
        return false;
    }

#ifdef ELF32_DEBUG
    kprintf("Entry point @ %08x\n", ehdr->e_entry);
    kprintf("sec_num=%d, sec_entsize=%d\n", ehdr->e_shnum, ehdr->e_shentsize);
#endif

    // First of all check if the sections are not in the user space
    // boundaries.
    bool isValid = true;
    this->forEachSection([&](const Elf32_Shdr* sec) {
        const uintptr_t start = sec->sh_addr;
        const uintptr_t end = start + sec->sh_size;

        // skip section if he is empty, or not marked for allocation.
        if (0 == sec->sh_size || !(sec->sh_flags & SHF_ALLOC)) {
            return;
        }

        if (!platform_is_in_userspace_range(start, end)) {
            isValid = false;
        }
    });

    if (!isValid) {
        return false;
    }

    return true;
}

uintptr_t ElfLoader::getEntryPoint(void) const {
    const Elf32_Ehdr* ehdr = (Elf32_Ehdr *) _buffer;
    return ehdr->e_entry;
}

bool ElfLoader::loadSections(ProcessMemoryMap& memoryMap) {

    bool success = true;

    this->forEachSection([&](const Elf32_Shdr* sec) {
        const uintptr_t start = sec->sh_addr;
        const uintptr_t end = start + sec->sh_size;
        const char* name = this->getStringByIndex((int) sec->sh_name);

        // skip section if he is empty, or not marked for allocation.
        if (0 == sec->sh_size || !(sec->sh_flags & SHF_ALLOC)) {
            return;
        }

        // TODO: when mprotect is implemented, remove the VM_PROT_WRITE here and call mprotect after copying the data.
        uint32_t prot = VM_PROT_READ | VM_PROT_WRITE;
        if (sec->sh_flags & SHF_WRITE) {
            prot |= VM_PROT_WRITE;
        }

        if (sec->sh_flags & SHF_EXECINSTR) {
            prot |= VM_PROT_EXEC;
        }

        kprintf("Adding section '%s' @ %08x... ", name, start);
        if (!memoryMap.createMemoryRegion(name, start, end, (vm_prot_t) prot, true)) {
            success = false;
            kputs("Failed.\n");
            return;
        }

        kputs("Done.\n");

        memoryMap.runInScope([&]() {
            if (SHT_PROGBITS == sec->sh_type) {
                const char* from = (const char*)_buffer + sec->sh_offset;
                void* to = (void*)start;
                memcpy(to, from, sec->sh_size);
            } else if (SHT_NOBITS == sec->sh_type) {
                memset((void *) start, 0, sec->sh_size);
            }
        });
    });

    return success;
}

const char* ElfLoader::getStringByIndex(int index) const {
    const Elf32_Ehdr* ehdr = (Elf32_Ehdr *) _buffer;
    const Elf32_Shdr* strtabsection = (const Elf32_Shdr *) (((uintptr_t)ehdr) + ehdr->e_shoff) + ehdr->e_shstrndx;

    return (const char *) (((uintptr_t)ehdr) + strtabsection->sh_offset + index);
}

uintptr_t ElfLoader::getEndAddress(void) const {
    const Elf32_Shdr* bss = this->getSectionByName(".bss");
    if (!bss) {
        return 0;
    }

    return bss->sh_addr + bss->sh_size;
}

const Elf32_Shdr *ElfLoader::getSectionByName(const char *name) const {
    const Elf32_Shdr* found = NULL;

    this->forEachSection([&](const Elf32_Shdr* sec) {
        const char* sectionName = this->getStringByIndex((int) sec->sh_name);
        if (0 == strcmp(sectionName, name)) {
            found = sec;
        }
    });

    return found;
}

