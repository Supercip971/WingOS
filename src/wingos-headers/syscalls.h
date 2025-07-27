#pragma once 


#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct SyscallInterface {
    uint32_t id; 
    uint32_t _zero; // This is a zero field to align the structure
    uintptr_t arg1;
    uintptr_t arg2;
    uintptr_t arg3;
    uintptr_t arg4;
    uintptr_t arg5;
    uintptr_t arg6;
} SyscallInterface;


// ------- SYSCALL DEBUG ----

// cause a no-op in release mode, because it is really unsafe 
#define SYSCALL_DEBUG_LOG_ID 0x00000000
typedef struct SyscallDebug 
{
    char *message;
} SyscallDebug;

static inline SyscallInterface syscall_debug_encode(SyscallDebug debug)
{
    return SyscallInterface{SYSCALL_DEBUG_LOG_ID, 0, (uintptr_t)debug.message, 0, 0, 0, 0, 0};
}

static inline SyscallDebug syscall_debug_decode(SyscallInterface interface)
{
    return SyscallDebug{(char *)interface.arg1};
}

// ------- SYSCALL PHYSICAL MEM OWN ----

#define SYSCALL_PHYSICAL_MEM_OWN_ID 0x00000001

typedef struct SyscallMemOwn
{
    uint64_t target_space_handle;
    size_t size;
    size_t addr; // the address of the memory, if 0, it will be allocated by the kernel, will be set to the allocated address
    uint64_t returned_handle;
} SyscallMemOwn;

static inline SyscallInterface syscall_physical_mem_own_encode(SyscallMemOwn* create)
{
    return SyscallInterface{SYSCALL_PHYSICAL_MEM_OWN_ID, 0,(uintptr_t) create, 0, 0, 0, 0, 0};
}

static inline SyscallMemOwn syscall_physical_mem_own_decode(SyscallInterface interface)
{
    SyscallMemOwn *create = (SyscallMemOwn *)interface.arg1;
    return *create;
}

// ------- SYSCALL MAPPING CREATE ----

#define SYSCALL_MAPPING_CREATE_ID 0x00000002

typedef struct SyscallMap
{
    uint64_t target_space_handle;
    size_t start;
    size_t end;
    uint64_t physical_mem_handle; // the handle of the physical memory asset
    uint64_t flags;
    uint64_t returned_handle; // the handle of the created mapping
} SyscallMap;

static inline SyscallInterface syscall_map_encode(SyscallMap* create)
{
    return SyscallInterface{SYSCALL_MAPPING_CREATE_ID, 0, (uintptr_t)create, 0, 0, 0, 0, 0};
}

static inline SyscallMap syscall_map_decode(SyscallInterface interface)
{
    SyscallMap *create = (SyscallMap *)interface.arg1;
    return *create;
}

// ------- SYSCALL TASK CREATE ----

#define SYSCALL_TASK_CREATE_ID 0x00000003

typedef struct SyscallTaskCreate
{
    uint64_t target_space_handle;
    uint64_t launch; // the launch parameters for the task
    
    uint64_t args[4]; // the arguments for the task, can be used to pass data to the task
    uint64_t returned_handle;
} SyscallTaskCreate;

static inline SyscallInterface syscall_task_create_encode(SyscallTaskCreate* create)
{
    return SyscallInterface{SYSCALL_TASK_CREATE_ID, 0, (uintptr_t)create, 0, 0, 0, 0, 0};
}

static inline SyscallTaskCreate syscall_task_create_decode(SyscallInterface interface)
{
    SyscallTaskCreate *create = (SyscallTaskCreate *)interface.arg1;
    return *create;
}

// ------- SYSCALL SPACE CREATE ----

#define SYSCALL_SPACE_CREATE_ID 0x00000004
typedef struct SyscallSpaceCreate
{
    uint64_t parent_space_handle; // the handle of the parent space
    uint64_t flags; // flags for the space creation
    uint64_t rights; // rights for the space creation
    uint64_t returned_handle; // the handle of the created space
} SyscallSpaceCreate;

static inline SyscallInterface syscall_space_create_encode(SyscallSpaceCreate* create)
{
    return SyscallInterface{SYSCALL_SPACE_CREATE_ID, 0, (uintptr_t)create, 0, 0, 0, 0, 0};
}

static inline SyscallSpaceCreate syscall_space_create_decode(SyscallInterface interface)
{
    SyscallSpaceCreate *create = (SyscallSpaceCreate *)interface.arg1;
    return *create;
}

// ------- SYSCALL ASSET RELEASE ----

#define SYSCALL_ASSET_RELEASE_ID 0x00000005

typedef struct SyscallAssetRelease
{
    uint64_t space_handle; // the handle of the space, if null, the asset will be released from the kernel
    uint64_t asset_handle; // the handle of the asset to release
    void* addr; // the address of the asset if it is memory 
} SyscallAssetRelease;

static inline SyscallInterface syscall_asset_release_encode(SyscallAssetRelease* release)
{
    return SyscallInterface{SYSCALL_ASSET_RELEASE_ID, 0, (uintptr_t)release, 0, 0, 0, 0, 0};
}

static inline SyscallAssetRelease syscall_asset_release_decode(SyscallInterface interface)
{
    SyscallAssetRelease *release = (SyscallAssetRelease *)interface.arg1;
    return *release;
}


// ------- SYSCALL TASK LAUNCH ----

#define SYSCALL_TASK_LAUNCH_ID 0x00000006

typedef struct SyscallTaskLaunch
{
    uint64_t target_space_handle; // the handle of the space to launch the task in
    uint64_t task_handle; // the handle of the task to launch
    uint64_t args[4]; // the arguments for the task, can be used to pass data to the task
} SyscallTaskLaunch;

static inline SyscallInterface syscall_task_launch_encode(SyscallTaskLaunch* launch)
{
    return SyscallInterface{SYSCALL_TASK_LAUNCH_ID, 0, (uintptr_t)launch, 0, 0, 0, 0, 0};
}

static inline SyscallTaskLaunch syscall_task_launch_decode(SyscallInterface interface)
{
    SyscallTaskLaunch *launch = (SyscallTaskLaunch *)interface.arg1;
    return *launch;
}



#ifdef __cplusplus
}
#endif
