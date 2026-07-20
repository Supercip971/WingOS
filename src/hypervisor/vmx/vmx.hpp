#pragma once

#include <stdint.h>

#include "hw/mem/addr_space.hpp"

struct VmxSystemState
{

    PhysAddr vmx_region;

    void *vmx_region_mapped;

    uintptr_t cr0_fixed_to_0 = 0;
    uintptr_t cr4_fixed_to_0 = 0;

    uintptr_t cr0_fixed_to_1 = 0;
    uintptr_t cr4_fixed_to_1 = 0;
    uint32_t vmx_version = 0;
};

struct VmxMachineEnv
{
};
