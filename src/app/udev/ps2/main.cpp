#include <stddef.h>
#include <string.h>
#include <sys/types.h>

#include "libcore/fmt/fmt_str.hpp"
#include "libcore/str_writer.hpp"

#include "app/udev/ps2/controller.hpp"
#include "app/udev/ps2/keyboard.hpp"
#include "app/udev/ps2/mouse.hpp"
#include "arch/generic/syscalls.h"
#include "iol/ports.hpp"
#include "iol/wingos/execute.hpp"
#include "iol/wingos/space.hpp"
#include "iol/wingos/syscalls.h"
#include "libcore/alive.hpp"
#include "libcore/fmt/log.hpp"
#include "mcx/mcx.hpp"
#include "protocols/compositor/compositor.hpp"
#include "protocols/compositor/window.hpp"
#include "protocols/init/init.hpp"
#include "protocols/pipe/pipe.hpp"
#include "protocols/vfs/vfs.hpp"
#include "wingos-headers/startup.hpp"
#include "wingos-headers/syscalls.h"


// source: derived from brutal OS but 
// I wrote the brutal PS2 code 


int main(int, char **)
{
    
    
    log::log$("started ps2 service");
    

    Ps2::Controller controller = Ps2::Controller();
    Ps2::Ps2Keyboard keyboard(controller);
    keyboard.init();
    Ps2::Mouse mouse(controller);
    mouse.init();

    controller.flush();
    while (true)
    {
        
        if(mouse.handle_event())
        {
            auto ev_res = mouse.poll_event();
            while (!ev_res.is_error())
            {
                log::log$("mouse event: dx={} dy={}", ev_res.unwrap().offx, ev_res.unwrap().offy);
                ev_res = mouse.poll_event();
            }

        }

        if (keyboard.packet_handle())
        {
            auto ev_res = keyboard.poll_event();
            while (!ev_res.is_error())
            {
                log::log$("keyboard event: key={} down={}", ev_res.unwrap().key, ev_res.unwrap().down);
                ev_res = keyboard.poll_event();
            }
        }

    }
}