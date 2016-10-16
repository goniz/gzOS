#include <cassert>
#include <cstdio>
#include <lib/mm/pmap.h>
#include <lib/mm/vm_object.h>
#include <lib/mm/vm_pager.h>
#include <platform/kprintf.h>
#include <lib/primitives/align.h>
#include "ProcessMemoryMap.h"

ProcessMemoryMap::ProcessMemoryMap(asid_t id)
        : _map(vm_map_new((vm_map_type_t) PMAP_USER, id))
{
    assert(nullptr != _map);
}

ProcessMemoryMap::~ProcessMemoryMap(void)
{
    if (_map) {
        vm_map_delete(_map);
    }
}

void ProcessMemoryMap::activate(void) const {
    set_active_vm_map(_map);
}

VirtualMemoryRegion* ProcessMemoryMap::createMemoryRegion(const char *name,
                                                          vm_addr_t start, vm_addr_t end,
                                                          vm_prot_t prot,
                                                          bool plain)
{
    if (_regions.get(name)) {
        return nullptr;
    }

    if (!_regions.put(name, VirtualMemoryRegion(*this, name, start, end, prot, plain))) {
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

bool ProcessMemoryMap::is_kernel_space(void) const {
    return _map->pmap.type == PMAP_KERNEL;
}

bool ProcessMemoryMap::is_user_space(void) const {
    return _map->pmap.type == PMAP_USER;
}

VirtualMemoryRegion::VirtualMemoryRegion(ProcessMemoryMap& parent,
                                         const char *name,
                                         vm_addr_t start, vm_addr_t end,
                                         vm_prot_t prot,
                                         bool plain)
    : _parent(parent),
      _name(name),
      _pool(std::make_unique<malloc_pool_t>()),
      _header(nullptr),
      _data(nullptr),
      _footer(nullptr)
{
    char tmp[8];
    sprintf(tmp, "%d", _parent._map->pmap.asid);
    _poolName.reserve(64);
    _poolName.append(name);
    _poolName.append("#");
    _poolName.append(tmp);

    *_pool = MALLOC_INITIALIZER(_pool, _poolName.c_str());
    kmalloc_init(_pool.get());

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

    if (!plain) {
        _parent.runInScope([&]() {
            kmalloc_add_arena(_pool.get(), (void *) start, end - start);
        });
    }
}

VirtualMemoryRegion::~VirtualMemoryRegion(void)
{
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
    std::swap(_name, other._name);
    std::swap(_poolName, other._poolName);
    std::swap(_pool, other._pool);
    std::swap(_header, other._header);
    std::swap(_data, other._data);
    std::swap(_footer, other._footer);

    kmalloc_set_description(_pool.get(), _poolName.c_str());
}

void *VirtualMemoryRegion::allocate(size_t size) const {
    void* ptr = nullptr;

    _parent.runInScope([&]() {
        ptr = kmalloc(_pool.get(), size, M_NOWAIT);
    });

    return ptr;
}

void VirtualMemoryRegion::free(void *ptr) const {
    _parent.runInScope([&]() {
        kfree(_pool.get(), ptr);
    });
}

void VirtualMemoryRegion::mprotect(vm_prot_t prot) {
    if (!_data) {
        return;
    }

    vm_map_protect(_parent._map, _data->start, _data->end, prot);
}

bool VirtualMemoryRegion::extend(uintptr_t endAddr) {
    if (!_data) {
        return false;
    }

    // already mapped..
    if (endAddr >= _data->start && endAddr <= _data->end) {
        return true;
    }

    return vm_map_extend_entry(_parent._map, _data, endAddr) != 0;
}
