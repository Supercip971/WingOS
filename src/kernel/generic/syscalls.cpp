#include "syscalls.hpp"

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
    if (mem_own->target_space_handle != 0)
    {
        // TODO:
        log::warn$("syscall: physical memory ownership is not implemented for target space handle, using current task space");
    }

    auto space = Cpu::current()->currentTask()->space();
    if (space == nullptr)
    {
        return core::Result<size_t>::error("no current space");
    }

    auto asset = try$(asset_create_memory(space, {
                                                     .size = mem_own->size,
                                                     .addr = mem_own->addr,
                                                     .lower_half = true, // TODO: implement lower half allocation
                                                 }));

    mem_own->addr = asset.asset->memory.addr;

    return core::Result<size_t>::success((uint64_t)asset.handle);

    return {};
}

core::Result<uintptr_t> ksyscall_map(SyscallMap *map)
{
    if (map->target_space_handle != 0)
    {

        log::warn$("syscall: mapping is not implemented for target space handle, using current task space");
    }
    auto space = Cpu::current()->currentTask()->space();
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

    return core::Result<size_t>::success((uint64_t)asset.handle);
}

core::Result<size_t> ksyscall_task_create(SyscallTaskCreate *task_create)
{
    if (task_create->target_space_handle != 0)
    {
        log::warn$("syscall: task creation is not implemented for target space handle, using current task space");
    }

    auto space = Cpu::current()->currentTask()->space();
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
    auto parent_space = Cpu::current()->currentTask()->space();
    if (parent_space == nullptr)
    {
        return core::Result<size_t>::error("no current space");
    }

    if(args->parent_space_handle != 0)
    {
        log::warn$("syscall: space creation is not implemented for target space handle, using current task space");
    }

    auto asset = try$(space_create(parent_space, args->flags, args->rights));


    return core::Result<size_t>::success((uint64_t)asset.handle);
}

core::Result<size_t> syscall_handle(SyscallInterface syscall)
{
    switch (syscall.id)
    {
    case SYSCALL_DEBUG_LOG_ID:
    {
        auto debug = syscall_debug_decode(syscall);
        log::log$("DEBUG: {}", debug.message);
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
    default:
        return {"Unknown syscall ID"};
    }
}
