#pragma once 
#include <stddef.h>
#include <stdint.h>
#include "mcx/mcx.hpp"

struct StartupInfo {
    mcx::MachineContext machine_context_optional;
    uintptr_t stdin_handle; 
    uintptr_t stdout_handle;
    uintptr_t stderr_handle;

    uintptr_t cwd_handle; 
    uintptr_t root_dir_handle;
    size_t argc; 
    char** argv;
};

