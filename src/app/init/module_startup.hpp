#pragma once 

#include "libcore/result.hpp"
#include "mcx/mcx.hpp"

core::Result<void> startup_module(mcx::MachineContext *context);

core::Result<void> service_startup_callback( core::Str service_name);
