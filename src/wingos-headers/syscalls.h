#pragma once 


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

#ifdef __cplusplus
}
#endif
