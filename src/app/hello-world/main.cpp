#include "protocols/hi/human_interface.hpp"
#include "arch/generic/syscalls.h"
#include "iol/wingos/space.hpp"
#include "iol/wingos/syscalls.h"
#include "libcore/alive.hpp"
#include "libcore/fmt/log.hpp"
#include "libcore/result.hpp"
#include "mcx/mcx.hpp"
#include "protocols/compositor/compositor.hpp"
#include "protocols/compositor/window.hpp"
#include "protocols/init/init.hpp"
#include "protocols/pipe/pipe.hpp"
#include "protocols/vfs/vfs.hpp"
#include "wingos-headers/syscalls.h"

int main(int, char **)
{

    log::log$("Hello world from an application !");
    log::log$("Wingos is a microkernel based OS! made with <3");

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

    prot::HIConnection hi_conn = prot::HIConnection::connect().unwrap();
    hi_conn.start_listen().unwrap();




    while (true)
    {
        hi_conn.event_queue().update_event();
        core::Result<prot::HIEvent> event_res = hi_conn.event_queue().poll_event();
        while (!event_res.is_error())
        {
            prot::HIEvent event = event_res.unwrap();
            switch (event.type)
            {
            case prot::HI_EVENT_TYPE_MOUSE:
                log::log$("mouse event: dx={} dy={} buttons={}", event.mouse.dx, event.mouse.dy, event.mouse.buttons);
                break;
            case prot::HI_EVENT_TYPE_KEYBOARD:
                log::log$("keyboard event: key={} pressed={}", event.keyboard.keycode, event.keyboard.pressed);
                break;
            default:
                log::log$("unknown event type: {}", (int)event.type);
                break;
            }
            event_res = hi_conn.event_queue().poll_event();
        }
    };
}