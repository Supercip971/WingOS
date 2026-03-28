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


    float x = 0;
    float y = 0;
    float dx = 1;
    float dy = 1;
    wgfx::initialize_platform();


    auto window = wgfx::PlatformWindow::create_native(wgfx::BackendsKinds::BACKEND_KIND_RASTER).copied();

    window->attach();
    float i = 0;
    (void)i;
    while(true)
    {
        auto frame=  window->create_frame();

        log::log$("ran frame");

        auto col = wgfx::CompositeColor::fromOklch((float)63.7 / 100, 0.237, 0.5);



        frame->clear(col);

        frame->drawRect(x, y, 256, 256, wgfx::CompositeColor::fromOklch((float)63.7/100, 0, 0));

        x += dx;
        y += dy;

        if(x >= ((long)window->width() - 256) || x <= 0)
        {
            dx = -dx;
        }
        if(y >= (long)(window->height() - 256) || y <= 0)
        {
            dy = -dy;
        }


        window->end_frame(frame);
    }



}
