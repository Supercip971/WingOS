#pragma once
#include "libcore/fmt/log.hpp"
#include "libcore/result.hpp"
#include "libelf/elf.hpp"
#include "mcx/mcx.hpp"

core::Result<size_t> execute_module(mcx::MachineContext *ctx, elf::ElfLoader loaded);

core::Result<void> startup_module(mcx::MachineContext *context);

core::Result<void> service_startup_callback(core::Str service_name);

VirtRange map_mcx_address(mcx::MemoryRange range);