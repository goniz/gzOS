#ifndef GZOS_PROCESSMEMORYMAP_H
#define GZOS_PROCESSMEMORYMAP_H

#ifdef __cplusplus
#include <lib/primitives/hashmap.h>
#include <lib/mm/vm.h>
#include <lib/mm/pmap.h>
#include <lib/mm/vm_map.h>
#include <lib/malloc/malloc.h>
#include <memory>
#include <string>

class VirtualMemoryRegion;
class ProcessMemoryMap;

class ProcessMemoryMap
{
    friend class VirtualMemoryRegion;

public:
    ProcessMemoryMap(void);
    ~ProcessMemoryMap(void);

    ProcessMemoryMap(ProcessMemoryMap&&) = delete;
    ProcessMemoryMap(const ProcessMemoryMap&) = delete;

    VirtualMemoryRegion* createMemoryRegion(const char* name, vm_addr_t start, vm_addr_t end, vm_prot_t prot, bool plain = false);
    void destroyMemoryRegion(const char* name);

    VirtualMemoryRegion* get(const char* name) const;

    void activate(void) const;

    template<typename TFunc>
    void runInScope(TFunc&& func) const {
        auto* old = get_user_vm_map();
        vm_map_activate(_map);

        try { func(); } catch (...) { }

        vm_map_activate(old);
    }

private:
    vm_map_t* _map;
    HashMap<const char*, VirtualMemoryRegion> _regions;
};

class VirtualMemoryRegion
{
public:
    VirtualMemoryRegion(ProcessMemoryMap& parent, const char* name, vm_addr_t start, vm_addr_t end, vm_prot_t prot, bool plain = true);
    ~VirtualMemoryRegion(void);

    VirtualMemoryRegion(VirtualMemoryRegion&& other);
    VirtualMemoryRegion(const VirtualMemoryRegion&) = delete;

    bool extend(uintptr_t endAddr);
    void mprotect(vm_prot_t prot);
    void* allocate(size_t size) const;
    void free(void* ptr) const;

private:
    ProcessMemoryMap& _parent;
    const char* _name;
    std::string _poolName;
    malloc_pool_t* _pool;
    vm_map_entry_t* _header;
    vm_map_entry_t* _data;
    vm_map_entry_t* _footer;
};


#endif //cplusplus
#endif //GZOS_PROCESSMEMORYMAP_H
