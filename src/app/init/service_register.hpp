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

fc::Result<IpcServerHandle> service_get(fc::Str const &name, uint64_t major = 1, uint64_t minor = 0);

fc::Result<void> service_register(uint64_t endpoint, fc::Str const &name, uint64_t major = 1, uint64_t minor = 0);
