#pragma once

// Forward declarations to avoid circular include with `space.hpp`.

// <!> create two object: one for the sender connection and one for the receiver connection

#include "kernel/generic/asset_types.hpp"

#include "kernel/generic/task.hpp"

struct Space;

struct AssetMemory : public Asset
{

    static constexpr size_t IDENT = AssetKind::OBJECT_KIND_MEMORY;
    size_t size;
    size_t addr;
    bool allocated;

    AssetMemory(size_t size_value, size_t addr_value, bool allocated_value)
        : Asset(AssetKind::OBJECT_KIND_MEMORY), size(size_value), addr(addr_value), allocated(allocated_value) {}
};

struct AssetMapping : public Asset
{
    static constexpr size_t IDENT = AssetKind::OBJECT_KIND_MAPPING;
    size_t start;
    size_t end;
    AssetRef<AssetMemory> physical_mem; // the physical memory that this mapping is based on
    bool writable;
    bool executable;

    AssetMapping(size_t start_value, size_t end_value, AssetRef<AssetMemory> physical_mem_value, bool writable_value, bool executable_value)
        : Asset(AssetKind::OBJECT_KIND_MAPPING), start(start_value), end(end_value), physical_mem(physical_mem_value), writable(writable_value), executable(executable_value) {}
};

using AssetTask = kernel::Task;
