#include <cassert>
#include <cstdio>
#include <lib/mm/pmap.h>
#include <lib/mm/vm_object.h>
#include <lib/mm/vm_pager.h>
#include <platform/kprintf.h>
#include <lib/primitives/align.h>
#include <lib/primitives/interrupts_mutex.h>
#include <lib/mm/physmem.h>
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
        : _map(nullptr)
{
    _map = vm_map_new();
    assert(nullptr != _map);
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
                                                          vm_prot_t prot,
                                                          bool plain)
{
    if (_regions.get(name)) {
        return nullptr;
    }

    VirtualMemoryRegion region(*this, name, start, end, prot, plain);
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

VirtualMemoryRegion::VirtualMemoryRegion(ProcessMemoryMap& parent,
                                         const char *name,
                                         vm_addr_t start, vm_addr_t end,
                                         vm_prot_t prot,
                                         bool plain)
    : _parent(parent),
      _name(name),
      _pool(nullptr),
      _header(nullptr),
      _data(nullptr),
      _footer(nullptr)
{
    char tmp[16];
    sprintf(tmp, "%d", _parent._map->pmap->asid);
    _poolName.reserve(64);
    _poolName.append(name);
    _poolName.append("#");
    _poolName.append(tmp);

    InterruptsMutex guard(true);

    if (!plain) {
        _pool = (malloc_pool_t*) kmalloc(mp_memory_map, sizeof(*_pool), M_ZERO);
        kmalloc_init(_pool);
        kmalloc_set_description(_pool, _poolName.c_str());
    }

    if (!plain) {
        _header = vm_map_add_entry(_parent._map, start - PAGESIZE, start, (vm_prot_t)VM_PROT_NONE);
        assert(NULL != _header);
        _header->object = vm_object_alloc();
    }

    _data = vm_map_add_entry(_parent._map, start, end, prot);
    assert(NULL != _data);
    _data->object = default_pager->pgr_alloc();

    if (!plain) {
        _footer = vm_map_add_entry(_parent._map, end, end + PAGESIZE, (vm_prot_t)VM_PROT_NONE);
        assert(NULL != _footer);
        _footer->object = vm_object_alloc();
    }

    if (_pool) {
        _parent.runInScope([&]() {
            kmalloc_add_arena(_pool, (void *) start, end - start);
        });
    }
}

VirtualMemoryRegion::~VirtualMemoryRegion(void)
{
    InterruptsMutex guard(true);

    if (_pool) {
        memset(_pool, 0, sizeof(*_pool));
        kfree(mp_memory_map, _pool);
    }

    if (_header) {
        vm_map_remove_entry(_parent._map, _header);
    }

    if (_data) {
        vm_map_remove_entry(_parent._map, _data);
    }

    if (_footer) {
        vm_map_remove_entry(_parent._map, _footer);
    }
}

VirtualMemoryRegion::VirtualMemoryRegion(VirtualMemoryRegion &&other)
    : _parent(other._parent),
      _name(nullptr),
      _poolName(),
      _pool(nullptr),
      _header(nullptr),
      _data(nullptr),
      _footer(nullptr)
{
    InterruptsMutex guard(true);

    std::swap(_name, other._name);
    std::swap(_poolName, other._poolName);
    std::swap(_pool, other._pool);
    std::swap(_header, other._header);
    std::swap(_data, other._data);
    std::swap(_footer, other._footer);

    if (_pool) {
        kmalloc_set_description(_pool, _poolName.c_str());
    }
}

void *VirtualMemoryRegion::allocate(size_t size) const {
    InterruptsMutex guard(true);
    void* ptr = nullptr;

    _parent.runInScope([&]() {
        ptr = kmalloc(_pool, size, M_NOWAIT | M_ZERO);
    });

    return ptr;
}

void VirtualMemoryRegion::free(void *ptr) const {
    InterruptsMutex guard(true);

    _parent.runInScope([&]() {
        kfree(_pool, ptr);
    });
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
