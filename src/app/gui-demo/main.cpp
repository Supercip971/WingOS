#include <libcore/fmt/log.hpp>


#include "gfx/backend.hpp"
#include "gfx/color.hpp"
#include "gfx/platform/app.hpp"
#include "gfx/platform/window.hpp"
#include "gfx/text/font.hpp"
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

    auto t = wgfx::Typeface::from_file("./font.ttf").copied();

    auto font = wgfx::Font::load_font(t, 96).copied();



    (void)t;

    float i = 0;
    (void)i;

    auto contour = font.shapes['@'].gfx_contour;

    for(size_t j = 0; j< contour->strokes.len(); j++)
    {
        switch(contour->strokes[j].a.action)
        {
            case wgfx::PathAction::GMOVE:
            log::log$("GMOVE");
                break;
            case wgfx::PathAction::GCURVE:
                log::log$("GCURVE");
                break;
            case wgfx::PathAction::GCUBIC_CURVE:
                log::log$("GCUBIC");
                break;
            default:
                log::log$("UNKNOWN: ({}, {})", (long)contour->strokes[j].a.pos.x, (long)contour->strokes[j].a.pos.y);
                break;
        }
        log::log$("start: ({}, {})", (long)contour->strokes[j].a.pos.x, (long)contour->strokes[j].a.pos.y);
        log::log$("end: ({}, {})", (long)contour->strokes[j].b.pos.x, (long)contour->strokes[j].b.pos.y);
        log::log$("control: ({}, {})", (long)contour->strokes[j].a.curve.control.x, (long)contour->strokes[j].a.curve.control.y);
        log::log$("control2: ({}, {})", (long)contour->strokes[j].a.cubic_curve.control2.x, (long)contour->strokes[j].a.cubic_curve.control2.y);
        log::log$("");
    }
    while(true)
    {
        auto frame=  window->create_frame();

        //log::log$("ran frame");


        auto col = wgfx::CompositeColor::fromOklch((float)63.7 / 100, 0.237, 0.5);



        frame->clear(col);

        //frame->drawRect(wgfx::GRect(x, y, 256, 256), wgfx::CompositeColor::fromOklch((float)63.7/100, 0, 0));

        frame->drawContour(font.shapes['@'].gfx_contour, wgfx::CompositeColor::fromOklch(63.7/100, 0, 0));

        for(size_t j = 0; j < 300; j++)
        {

            frame->drawContour(font.shapes['@'].gfx_contour, wgfx::CompositeColor::fromOklch(63.7/100, 0, 0));
            (void)j;

        }










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
