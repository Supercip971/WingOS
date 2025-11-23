#pragma once 

#include <stdint.h>
#include "iol/wingos/space.hpp"
#include "libcore/result.hpp"
#include "libelf/elf.hpp"
#include "wingos-headers/startup.hpp"


core::Result<size_t> execute_program_from_mem(Wingos::Space& subspace, elf::ElfLoader elf_program, StartupInfo const &args);

core::Result<size_t> execute_program_from_path(Wingos::Space& subspace, const core::Str & path, StartupInfo const & args);

