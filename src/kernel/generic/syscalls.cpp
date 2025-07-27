#include "syscalls.hpp"

#include "arch/x86_64/paging.hpp"

#include "kernel/generic/asset.hpp"
#include "kernel/generic/context.hpp"
#include "kernel/generic/cpu.hpp"
#include "kernel/generic/task.hpp"
#include "libcore/fmt/log.hpp"
#include "wingos-headers/asset.h"
#include "wingos-headers/syscalls.h"

template <typename T>
core::Result<T *> syscall_check_ptr(uintptr_t ptr)
{
    if (ptr == 0)
    {
        return core::Result<T *>::error("null pointer");
    }

    auto tsk = Cpu::current()->currentTask();
    if (tsk == nullptr)
    {
        return core::Result<T *>::error("no current task");
    }

    try$(tsk->vmm_space().verify(ptr, sizeof(T)));

    return reinterpret_cast<T *>(ptr);
}

core::Result<uintptr_t> ksyscall_mem_own(SyscallMemOwn *mem_own)
{
    Space *space = nullptr;

    if (mem_own->target_space_handle != 0)
    {
        space = try$(Space::space_by_handle(mem_own->target_space_handle));
    }
    else
    {
        space = Cpu::current()->currentTask()->space();
    }

    if (space == nullptr)
    {
        return core::Result<size_t>::error("no current space");
    }

    auto asset = try$(asset_create_memory(space, {
                                                     .size = mem_own->size,
                                                     .addr = mem_own->addr,
                                                     .lower_half = false, // TODO: implement lower half allocation
                                                 }));

    mem_own->addr = asset.asset->memory.addr;

    mem_own->returned_handle = asset.handle;

    return core::Result<size_t>::success((uint64_t)asset.handle);

    return {};
}

core::Result<uintptr_t> ksyscall_map(SyscallMap *map)
{
    Space *space = nullptr;
    bool need_invalidate = false;
    if (map->target_space_handle != 0)
    {
        space = try$(Space::space_by_handle(map->target_space_handle));
    }
    else
    {
        space = Cpu::current()->currentTask()->space();
        need_invalidate = true;
    }

    if (space == nullptr)
    {
        return core::Result<size_t>::error("no current space");
    }

    auto asset = try$(asset_create_mapping(space, {
                                                      .start = map->start,
                                                      .end = map->end,
                                                      .physical_mem = try$(Asset::by_handle(space, map->physical_mem_handle)),
                                                      .writable = (map->flags & ASSET_MAPPING_FLAG_WRITE) != 0,
                                                      .executable = (map->flags & ASSET_MAPPING_FLAG_EXECUTE) != 0,
                                                  }));

    if (need_invalidate)
    {
        for (size_t i = map->start; i < map->end; i += arch::amd64::PAGE_SIZE)
        {
            VmmSpace::invalidate_address(VirtAddr(i));
        }
    }

    return core::Result<size_t>::success((uint64_t)asset.handle);
}

core::Result<size_t> ksyscall_task_create(SyscallTaskCreate *task_create)
{
    Space *space = nullptr;
    if (task_create->target_space_handle != 0)
    {
        space = try$(Space::space_by_handle(task_create->target_space_handle));
    }
    else
    {
        space = Cpu::current()->currentTask()->space();
    }

    if (space == nullptr)
    {
        return core::Result<size_t>::error("no current space");
    }

    auto asset = try$(asset_create_task(space, (AssetTaskCreateParams){
                                                   .launch = {
                                                       .entry = (void *)task_create->launch,
                                                       .user = true,
                                                   },
                                               }));

    return core::Result<size_t>::success((uint64_t)asset.handle);
}

core::Result<size_t> ksyscall_space_create(SyscallSpaceCreate *args)
{
    Space *space = nullptr;
    if (args->parent_space_handle != 0)
    {
        space = try$(Space::space_by_handle(args->parent_space_handle));
    }
    else
    {
        space = Cpu::current()->currentTask()->space();
    }

    if (space == nullptr)
    {
        return core::Result<size_t>::error("no current space");
    }

    auto asset = try$(space_create(space, args->flags, args->rights));

    return core::Result<size_t>::success((uint64_t)asset.handle);
}
core::Result<size_t> ksyscall_mem_release(SyscallAssetRelease *release)
{

    auto space = Cpu::current()->currentTask()->space();
    if (space == nullptr)
    {
        return core::Result<size_t>::error("no current space");
    }
    Asset *phys_mem = nullptr;
    Asset *virt_mem = nullptr;
    for (size_t i = 0; i < space->assets.len(); i++)
    {
        if (space->assets[i].asset->kind == OBJECT_KIND_MAPPING && space->assets[i].asset->mapping.start == (uintptr_t)release->addr)
        {

            virt_mem = space->assets[i].asset;
            phys_mem = space->assets[i].asset->mapping.physical_mem;
            break;
        }
    }
    if (phys_mem == nullptr || virt_mem == nullptr)
    {
        return core::Result<size_t>::error("no memory mapping found for address");
    }

    if (phys_mem->kind != OBJECT_KIND_MEMORY)
    {
        return core::Result<size_t>::error("physical memory asset is not a memory asset");
    }

    if (virt_mem->kind != OBJECT_KIND_MAPPING)
    {
        return core::Result<size_t>::error("virtual memory asset is not a mapping asset");
    }

    asset_release(space, phys_mem);
    asset_release(space, virt_mem);

    return core::Result<size_t>::success(0);
}
core::Result<size_t> ksyscall_asset_release(SyscallAssetRelease *release)
{

    if (release->asset_handle == 0 && release->addr != nullptr)
    {
        return ksyscall_mem_release(release);
    }

    Space *space = nullptr;
    if (release->space_handle != 0)
    {
        space = try$(Space::space_by_handle(release->space_handle));
    }
    else
    {
        space = Cpu::current()->currentTask()->space();
    }

    if (space == nullptr)
    {
        return core::Result<size_t>::error("no current space");
    }

    auto asset = try$(Asset::by_handle(space, release->asset_handle));

    asset_release(space, asset);

    return core::Result<size_t>::success(0);
}
core::Result<size_t> syscall_handle(SyscallInterface syscall)
{
    switch (syscall.id)
    {
    case SYSCALL_DEBUG_LOG_ID:
    {
        auto debug = syscall_debug_decode(syscall);
        log::log("{}", debug.message);

        return core::Result<size_t>::success(0);
    }
    case SYSCALL_PHYSICAL_MEM_OWN_ID:
    {
        SyscallMemOwn *mem_own = try$(syscall_check_ptr<SyscallMemOwn>(syscall.arg1));

        return ksyscall_mem_own(mem_own);
    }
    case SYSCALL_MAPPING_CREATE_ID:
    {
        SyscallMap *map = try$(syscall_check_ptr<SyscallMap>(syscall.arg1));

        return ksyscall_map(map);
    }
    case SYSCALL_TASK_CREATE_ID:
    {
        SyscallTaskCreate *task_create = try$(syscall_check_ptr<SyscallTaskCreate>(syscall.arg1));
        return ksyscall_task_create(task_create);
    }
    case SYSCALL_SPACE_CREATE_ID:
    {
        SyscallSpaceCreate *space_create = try$(syscall_check_ptr<SyscallSpaceCreate>(syscall.arg1));
        return ksyscall_space_create(space_create);
    }
    case SYSCALL_ASSET_RELEASE_ID:
    {
        SyscallAssetRelease *asset_release = try$(syscall_check_ptr<SyscallAssetRelease>(syscall.arg1));
        return ksyscall_asset_release(asset_release);
    }
    default:
        return {"Unknown syscall ID"};
    }
}
