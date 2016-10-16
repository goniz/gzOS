#ifndef GZOS_ELFLOADER_H
#define GZOS_ELFLOADER_H

#ifdef __cplusplus
#include <cstddef>
#include "lib/kernel/proccess/ProcessMemoryMap.h"
#include "lib/kernel/elf/elf.h"

class ElfLoader
{
public:
    ElfLoader(const void *buffer, size_t size);
    ~ElfLoader(void) = default;

    ElfLoader(ElfLoader&&) = delete;
    ElfLoader(const ElfLoader&) = delete;

    bool sanityCheck(void) const;
    bool loadSections(ProcessMemoryMap& memoryMap);
    uintptr_t getEntryPoint(void) const;
    uintptr_t getEndAddress(void) const;

    const Elf32_Shdr* getSectionByName(const char* name) const;

private:
    const char *getStringByIndex(int index) const;

    template<typename TFunc>
    void forEachSection(TFunc&& func) const {
        const Elf32_Ehdr* ehdr = (Elf32_Ehdr *) _buffer;
        const Elf32_Shdr* sec = (Elf32_Shdr*)((uint32_t)ehdr->e_shoff + (uint32_t)ehdr);
        for (int i = 0;
             i < ehdr->e_shnum;
             i++, sec = (Elf32_Shdr*)(((uintptr_t)sec) + ehdr->e_shentsize))
        {
            if (SHT_NULL == sec->sh_type) {
                continue;
            }

            try { func(sec); } catch (...) { }
        }
    }

    const void* _buffer;
    size_t _size;
};

#endif //cplusplus
#endif //GZOS_ELFLOADER_H
