#pragma once 

#include <wingos-headers/syscalls.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

#define SPACE_SELF 0

#ifdef __x86_64

uintptr_t syscall_execute(uint32_t id, uintptr_t arg1, uintptr_t arg2, uintptr_t arg3, uintptr_t arg4, uintptr_t arg5, uintptr_t arg6);


#endif
#ifdef __cplusplus
}
#endif 
