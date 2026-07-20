#pragma once

#include "hw/mem/addr_space.hpp"

struct VmcsRegion
{
    void *mapped;
    PhysAddr paddr;
};

struct VmxSystemState;
VmcsRegion allocate_vmcs_region(VmxSystemState *state);
