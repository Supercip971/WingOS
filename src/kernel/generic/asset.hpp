#pragma once

#include "iol/mem_flags.h"

#include "wingos-headers/ipc.h"

#include "kernel/generic/ipc.hpp"
#include "kernel/generic/mem.hpp"
#include "kernel/generic/paging.hpp"
#include "kernel/generic/pmm.hpp"
#include "mcx/mcx.hpp"

// Forward declarations to avoid circular include with `space.hpp`.
struct Space;

// <!> create two object: one for the sender connection and one for the receiver connection
