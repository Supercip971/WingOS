#include "arch/generic/syscalls.h"
#include "iol/wingos/space.hpp"
#include "iol/wingos/syscalls.h"
#include "libcore/fmt/log.hpp"
#include "mcx/mcx.hpp"
#include "protocols/init/init.hpp"
#include "protocols/vfs/vfs.hpp"
#include "wingos-headers/syscalls.h"

int _main(mcx::MachineContext *)
{

    // attempt connection to open root file

    auto conn = prot::VfsConnection::connect().unwrap();



    auto b3 = conn.open_path(core::Str("/boot/config/init-services.json")).unwrap();

    auto data_asset = Wingos::Space::self().allocate_physical_memory(4096);

    size_t res = b3.read(data_asset, 0, 4096).unwrap();

    auto data_ptr = Wingos::Space::self().map_memory(data_asset, ASSET_MAPPING_FLAG_READ);

    log::log$("read {} bytes from /boot/config/init-services.json:", res);

    log::log$("{}", core::Str((const char *)data_ptr.ptr(), res));

    log::log$("hello world from vfs app!");

    while (true)
    {
    }
}