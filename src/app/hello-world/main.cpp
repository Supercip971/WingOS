#include "arch/generic/syscalls.h"
#include "iol/wingos/space.hpp"
#include "iol/wingos/syscalls.h"
#include "libcore/alive.hpp"
#include "libcore/fmt/log.hpp"
#include "mcx/mcx.hpp"
#include "protocols/compositor/compositor.hpp"
#include "protocols/compositor/window.hpp"
#include "protocols/init/init.hpp"
#include "protocols/vfs/vfs.hpp"
#include "wingos-headers/syscalls.h"

int main(int , char** )
{
    core::Alive alive {"hello-world"};


    // attempt connection to open root file

    /*
    auto conn = prot::VfsConnection::connect().unwrap();



    auto b3 = conn.open_path(core::Str("/boot/config/init-services.json")).unwrap();

    auto data_asset = Wingos::Space::self().allocate_physical_memory(4096);

    size_t res = b3.read(data_asset, 0, 4096).unwrap();

    auto data_ptr = Wingos::Space::self().map_memory(data_asset, ASSET_MAPPING_FLAG_READ);

    log::log$("read {} bytes from /boot/config/init-services.json:", res);

    log::log$("{}", core::Str((const char *)data_ptr.ptr(), res));
*/
    log::log$("hello world from vfs app!");

    auto wdw = prot::WindowConnection::create(true).unwrap();

    auto asset = wdw.get_framebuffer().unwrap();

    void *fb = asset.ptr();

    auto size = wdw.get_attribute_size().unwrap();

    log::log$("window size: {}x{}", size.width, size.height);

    size_t frame = 0;
    while (true)
    {

       // alive.tick();
        uint32_t r = frame % 256;
        uint32_t g = (frame / 256) % 256;
        uint32_t b = (frame / (256 * 256)) % 256;

        for (size_t i = 0; i < size.width * size.height; i++)
        {

            size_t x = (i % size.width) + frame ;
            size_t y = (i / size.width) + frame;
            b = x ^ y;
            r = (y * 2) ^ (x * 2);
            g = (y * 4) ^ (x * 4);

            ((uint32_t *)fb)[i] = 0xff000000 |
                                  (r << 16) | (g << 8) | (b);
        }
        wdw.swap_buffers();
        frame++;

    //    log::log$("swapped buffers: {}", frame);
    }
}