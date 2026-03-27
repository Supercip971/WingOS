#include <libcore/fmt/log.hpp>


#include "gfx/backend.hpp"
#include "gfx/color.hpp"
#include "gfx/platform/app.hpp"
#include "gfx/platform/window.hpp"
#include "libcore/result.hpp"


int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    log::log$("Hello, World!");


    wgfx::initialize_platform();


    auto window = wgfx::PlatformWindow::create_native(wgfx::BackendsKinds::BACKEND_KIND_RASTER).copied();

    window->attach();
    float i = 0;
    while(true)
    {
        auto frame=  window->create_frame();

        log::log$("ran frame");

        auto col = wgfx::CompositeColor::fromOklch((float)63.7 / 100, 0.237, i);

        i += 0.01;
        if(i > 100)
        {
            i = 0.0;
        }

        frame->clear(col);

        window->end_frame(frame);
    }



}
