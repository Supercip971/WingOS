
#include "libcore/fmt/log.hpp"
#include "libcore/result.hpp"
#include "protocols/vfs/vfs.hpp"

int main(int, char **)
{

    fmt::log$("Hello world from an application !");
    fmt::log$("Wingos is a microkernel based OS! made with <3");

    // attempt connection to open root file

    auto conn = prot::VfsConnection::connect().unwrap();

    auto b3 = conn.open_path(fc::Str("/boot/config/init-services.json")).unwrap();

    auto data_asset = Wingos::Space::self().allocate_physical_memory(4096);

    size_t res = b3.read(data_asset, 0, 4096).unwrap();

    auto data_ptr = Wingos::Space::self().map_memory(data_asset, ASSET_MAPPING_FLAG_READ);

    fmt::log$("read {} bytes from /boot/config/init-services.json:", res);

    fmt::log$("{}", fc::Str((const char *)data_ptr.ptr(), res));

    /*

    prot::HIConnection hi_conn = prot::HIConnection::connect().unwrap();
    hi_conn.start_listen().unwrap();

    while (true)
    {
        hi_conn.event_queue().update_event();
        fc::Result<prot::HIEvent> event_res = hi_conn.event_queue().poll_event();
        while (!event_res.is_error())
        {
            prot::HIEvent event = event_res.unwrap();
            switch (event.type)
            {
            case prot::HI_EVENT_TYPE_MOUSE:
                fmt::log$("mouse event: dx={} dy={} buttons={}", event.mouse.dx, event.mouse.dy, event.mouse.buttons);
                break;
            case prot::HI_EVENT_TYPE_KEYBOARD:
                fmt::log$("keyboard event: key={} pressed={}", event.keyboard.keycode, event.keyboard.pressed);
                break;
            default:
                fmt::log$("unknown event type: {}", (int)event.type);
                break;
            }
            event_res = hi_conn.event_queue().poll_event();
        }
        };*/
}
