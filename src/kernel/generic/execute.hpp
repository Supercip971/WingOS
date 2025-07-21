#pragma once 

#include "libcore/result.hpp"
#include "libelf/elf.hpp"

core::Result<void> start_module_execution(elf::ElfLoader loaded);


