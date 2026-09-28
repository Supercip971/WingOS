

#include "libcore/fmt/impl/asset_kind.hpp"
#include "protocols/hi/human_interface.hpp"

#include "iol/wingos/space.hpp"
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
    while (true)
    {
    }

    return 0;
}
