#pragma once 
#include "libelf/elf.hpp"
#include "libcore/result.hpp"
#include "mcx/mcx.hpp"
#include "libcore/fmt/log.hpp"

core::Result<size_t> execute_module(elf::ElfLoader loaded);

core::Result<void> startup_module(mcx::MachineContext *context);

core::Result<void> service_startup_callback( core::Str service_name);

VirtRange map_mcx_address(mcx::MemoryRange range);