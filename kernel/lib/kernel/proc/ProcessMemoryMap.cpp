#include <cassert>
#include <cstdio>
#include <lib/mm/pmap.h>
#include <lib/mm/vm_object.h>
#include <lib/mm/vm_pager.h>
#include <platform/kprintf.h>
#include <lib/primitives/align.h>
#include <lib/primitives/interrupts_mutex.h>
#include <lib/mm/physmem.h>
#include <platform/panic.h>
#include "ProcessMemoryMap.h"

static MALLOC_DEFINE(mp_memory_map, "ProcessMemoryMap malloc pools");

__attribute__((constructor,unused))
static void init_memory_pool()
{
    kmalloc_init(mp_memory_map);

    vm_page_t* page = pm_alloc(1);
    assert(page != NULL);

    kmalloc_add_arena(mp_memory_map, (void*)PG_VADDR_START(page), PG_SIZE(page));
}

ProcessMemoryMap::ProcessMemoryMap()
    : _map(vm_map_new()),
      _regions()
{
    assert(nullptr != _map);
}

ProcessMemoryMap::ProcessMemoryMap(const ProcessMemoryMap& other)
    : _map(nullptr),
      _regions()
{
    InterruptsMutex mutex(true);

    _map = vm_map_clone(other._map);
    assert(NULL != _map);

    other._regions.iterate([](any_t arg, char * key, any_t data) -> int {
        ProcessMemoryMap* self = (ProcessMemoryMap*)arg;
        const VirtualMemoryRegion* value = (const VirtualMemoryRegion*)data;

        if (!self->_regions.put(value->name(), VirtualMemoryRegion(*value, *self))) {
            panic("%s: failed to clone memory region %s\n", __func__, value->name());
        }

        return MAP_OK;
    }, this);
}

ProcessMemoryMap::~ProcessMemoryMap(void)
{
    InterruptsMutex guard(true);

    _regions.clear();

    if (_map) {
        vm_map_delete(_map);
    }

}

void ProcessMemoryMap::activate(void) const {
    vm_map_activate(_map);
}

VirtualMemoryRegion* ProcessMemoryMap::createMemoryRegion(const char *name,
                                                          vm_addr_t start, vm_addr_t end,
                                                          vm_prot_t prot)
{
    if (_regions.get(name)) {
        return nullptr;
    }

    VirtualMemoryRegion region(*this, name, start, end, prot);
    if (!_regions.put(name, std::move(region))) {
        return nullptr;
    }

    return _regions.get(name);
}

void ProcessMemoryMap::destroyMemoryRegion(const char *name) {
    _regions.remove(name);
}

VirtualMemoryRegion *ProcessMemoryMap::get(const char *name) const {
    return _regions.get(name);
}

VirtualMemoryRegion* ProcessMemoryMap::createMemoryRegionInRange(const char* name,
                                                                 vm_addr_t start, size_t size,
                                                                 vm_prot_t prot)
{
    vm_addr_t addr = 0;
    if (0 != vm_map_findspace(_map, start, size, &addr)) {
        return nullptr;
    }

    return this->createMemoryRegion(name, addr, addr + size, prot);
}

void ProcessMemoryMap::dump(void) const {
    vm_map_dump(_map);
}

VirtualMemoryRegion::VirtualMemoryRegion(ProcessMemoryMap& parent,
                                         const char *name,
                                         vm_addr_t start, vm_addr_t end,
                                         vm_prot_t prot)
    : _parent(parent),
      _name(std::string(name)),
      _data(nullptr)
{
    InterruptsMutex guard(true);

    _data = vm_map_add_entry(_parent._map, start, end, prot);
    assert(NULL != _data);

    _data->object = default_pager->pgr_alloc();
}

VirtualMemoryRegion::~VirtualMemoryRegion(void)
{
    InterruptsMutex guard(true);

    if (_data) {
        vm_map_remove_entry(_parent._map, _data);
    }
}

VirtualMemoryRegion::VirtualMemoryRegion(VirtualMemoryRegion&& other)
    : _parent(other._parent),
      _name(),
      _data(nullptr)
{
    InterruptsMutex guard(true);

    std::swap(_name, other._name);
    std::swap(_data, other._data);
}

VirtualMemoryRegion::VirtualMemoryRegion(const VirtualMemoryRegion& other, ProcessMemoryMap& parent)
    : _parent(parent),
      _name(other._name),
      _data(nullptr)
{
    InterruptsMutex guard(true);

    _data = vm_map_find_entry(_parent._map, other.startAddress());
    assert(NULL != _data);
}

void VirtualMemoryRegion::mprotect(vm_prot_t prot) {
    if (!_data) {
        return;
    }

    InterruptsMutex guard(true);
    vm_map_protect(_parent._map, _data->start, _data->end, prot);
}

bool VirtualMemoryRegion::extend(uintptr_t endAddr) {
    if (!_data) {
        return false;
    }

    // already mapped..
    if (endAddr >= _data->start && endAddr <= _data->end) {
//        kprintf("%s: %08x already mapped??\n", _name, endAddr);
        return true;
    }

//    kprintf("%s: Extending to %08x\n", _name, endAddr);

    InterruptsMutex guard(true);

    if (vm_map_resize(_parent._map, _data, endAddr)) {
        return true;
    }

    return false;
}

const char* VirtualMemoryRegion::name(void) const {
    return _name.c_str();
}

vm_addr_t VirtualMemoryRegion::startAddress(void) const {
    return _data->start;
}

size_t VirtualMemoryRegion::size(void) const {
    return _data->end - _data->start;
}
