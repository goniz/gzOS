#ifndef GZOS_ELFLOADER_H
#define GZOS_ELFLOADER_H

#ifdef __cplusplus
#include <cstddef>

class ElfLoader
{
public:
    ElfLoader(const void *buffer, size_t size);
    ~ElfLoader(void) = default;

    ElfLoader(ElfLoader&&) = delete;
    ElfLoader(const ElfLoader&) = delete;

    bool sanityCheck(void) const;

private:
    const void* _buffer;
    size_t _size;
};

#endif //cplusplus
#endif //GZOS_ELFLOADER_H
