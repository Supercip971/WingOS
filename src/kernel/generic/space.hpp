#pragma once 

#include <stdint.h>
#include "kernel/generic/paging.hpp"
#include "kernel/generic/task.hpp"
#include "libcore/ds/vec.hpp"
#include "libcore/lock/lock.hpp"



typedef enum {
    OBJECT_KIND_UNKNOWN = 0,
    OBJECT_KIND_MEMORY, // memory object, used for memory management
    OBJECT_KIND_MAPPING, // mapping object, used for memory mapping
    OBJECT_KIND_IPC, // IPC object, used for inter process communication
    OBJECT_KIND_SPACE,
    OBJECT_KIND_TASK,
} AssetKind;


struct Asset;

struct AssetPtr {
    Asset *asset;
    uint64_t handle; // the handle of the asset in the space
};

struct Space {

    size_t uid ;
    Asset* self;
    uint64_t space_handle; 
    Space* parent_space_handle; // the space that created this space
    VmmSpace vmm_space; // the virtual memory space of this space

    core::Vec<AssetPtr> assets;
};

struct Asset {
    core::Lock lock;

    size_t ref_count;
    AssetKind kind;
    union {
        struct {
            size_t size;
            size_t addr;
        } memory;

        struct {
            size_t start;
            size_t end;
            Asset *physical_mem; // the physical memory that this mapping is based on
            bool writable;
            bool executable;
        } mapping;


        Space* space;


        kernel::Task* task;
    };
};


core::Result<Asset *> space_create(Space* parent, uint64_t flags, uint64_t rights);
