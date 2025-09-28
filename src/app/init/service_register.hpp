#pragma once 


// /sys/init 


#include <stdlib.h>
#include <string.h>

#include "app/init/module_startup.hpp"
#include "hw/mem/addr_space.hpp"

#include "dev/pci/classes.hpp"
#include "dev/pci/pci.hpp"
#include "iol/wingos/ipc.hpp"
#include "iol/wingos/space.hpp"

#include "json/json.hpp"
#include "libcore/fmt/flags.hpp"
#include "libcore/fmt/log.hpp"
#include "libelf/elf.hpp"
#include "mcx/mcx.hpp"
#include "wingos-headers/asset.h"
#include "wingos-headers/ipc.h"


void startup_init_service(Wingos::IpcServer server);
