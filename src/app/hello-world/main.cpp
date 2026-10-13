

#include "libcore/fmt/impl/asset_kind.hpp"
#include "protocols/hi/human_interface.hpp"

#include "iol/wingos/space.hpp"
#include "iol/wingos/syscalls.h"
#include "libc/unistd.h"
#include "libcore/fmt/log.hpp"
#include "libcore/result.hpp"
#include "protocols/clock/clock.hpp"
#include "protocols/humming-bird/humming-bird.hpp"
#include "protocols/vfs/vfs.hpp"
#include "wingos-headers/asset.h"
#include "wingos-headers/syscalls.h"

int main(int, char **)
{

    fmt::log$("Hello world from an application !");
    fmt::log$("Wingos is a microkernel based OS! made with <3");

    auto conn = prot::HBConnection::connect().unwrap();

    fmt::log$("Humming Bird: connected to posix compatibility server, pid: {}", conn.get_posix_pid());

    size_t index = 0;
    Wingos::Space::self().iterate_through_assets([&](SyscallAssetInfo const &v)
                                                 { fmt::log$("Asset {}: handle={}", index++, v.returned_asset_handle);
                                                     fmt::log$("- kind: {}", assetKind2Str(v.returned_kind)); });

    auto vm_res = sys$create_vm(0);
    fmt::log$("VM created: handle={}", vm_res.result_vm);

    sys$vm_reg_write(0, vm_res.result_vm, 0, 0x0000000000000000, 0x0000000000000000);
    while (true)
    {
    }
    return 0;
}
