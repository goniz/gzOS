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
    ProcessMemoryMap(const ProcessMemoryMap& other);
    ~ProcessMemoryMap(void);

    ProcessMemoryMap(ProcessMemoryMap&&) = delete;

    // create a new memory region from start to end using prot
    VirtualMemoryRegion* createMemoryRegion(const char* name, vm_addr_t start, vm_addr_t end, vm_prot_t prot);
    // create a new memory region from the first empty space from start of size using prot
    VirtualMemoryRegion* createMemoryRegionInRange(const char* name, vm_addr_t start, size_t size, vm_prot_t prot);
    void destroyMemoryRegion(const char* name);

    VirtualMemoryRegion* get(const char* name) const;

    void activate(void) const;
    void dump(void) const;

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
    VirtualMemoryRegion(ProcessMemoryMap& parent, const char* name, vm_addr_t start, vm_addr_t end, vm_prot_t prot);
    VirtualMemoryRegion(const VirtualMemoryRegion& other, ProcessMemoryMap& parent);
    VirtualMemoryRegion(VirtualMemoryRegion&& other);
    ~VirtualMemoryRegion(void);


    const char* name(void) const;
    vm_addr_t startAddress(void) const;
    size_t size(void) const;
    bool extend(uintptr_t endAddr);
    void mprotect(vm_prot_t prot);

private:
    ProcessMemoryMap& _parent;
    std::string _name;
    vm_map_entry_t* _data;
};


#endif //cplusplus
#endif //GZOS_PROCESSMEMORYMAP_H
