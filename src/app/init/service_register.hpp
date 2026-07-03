#pragma once

#include <stdlib.h>
#include <string.h>

#include "iol/wingos/ipc.hpp"
#include "libcore/result.hpp"
#include "libcore/str.hpp"
#include "wingos-headers/ipc.h"

struct MachineContextShared
{
    uintptr_t framebuffer_addr;
    size_t framebuffer_width;
    size_t framebuffer_height;
};

void startup_init_service(Wingos::IpcServer server, MachineContextShared shared);

core::Result<IpcServerHandle> service_get(core::Str const &name, uint64_t major = 1, uint64_t minor = 0);

core::Result<void> service_register(uint64_t endpoint, core::Str const &name, uint64_t major = 1, uint64_t minor = 0);
