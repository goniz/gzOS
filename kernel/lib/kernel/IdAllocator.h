#ifndef GZOS_IDALLOCATOR_H
#define GZOS_IDALLOCATOR_H

#ifdef __cplusplus
#include <vector>

class IdAllocator {
public:
    IdAllocator(const size_t start, const size_t end);

    unsigned int allocate(void);
    void deallocate(const unsigned int id);
    bool isAllocated(const unsigned int id);

private:
    size_t _start;
    size_t _end;
    std::vector<bool> _allocated;
};

#endif //cplusplus
#endif //GZOS_IDALLOCATOR_H
