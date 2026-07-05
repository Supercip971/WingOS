#pragma once
#include "hw/mem/addr_space.hpp"

#include "libcore/result.hpp"
#include "libelf/elf.hpp"
#include "mcx/mcx.hpp"

fc::Result<size_t> execute_module(mcx::MachineContext *ctx, elf::ElfLoader loaded);

fc::Result<void> startup_module(mcx::MachineContext *context);

fc::Result<void> service_startup_callback(fc::Str service_name);

VirtRange map_mcx_address(mcx::MemoryRange range);
