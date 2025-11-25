#pragma once 

#include <stdint.h>
struct JumpState 
{
    #ifdef __x86_64
    uint64_t rbx;
    uint64_t rbp;
    uint64_t r12;
    uint64_t r13;
    uint64_t r14;
    uint64_t r15;
    uint64_t rsp;
    uint64_t rip;
    #else 
    #error "Unsupported architecture"
    #endif
};