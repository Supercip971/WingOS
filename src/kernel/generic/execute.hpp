#pragma once

#include "libcore/result.hpp"
#include "libelf/elf.hpp"
#include "mcx/mcx.hpp"

core::Result<void> start_module_execution(elf::ElfLoader loaded, mcx::MachineContext const *context);
