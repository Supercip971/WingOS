#include <libcore/fmt/log.hpp>


#include "gfx/backend.hpp"
#include "gfx/canvas/cmd.hpp"
#include "gfx/color.hpp"
#include "gfx/geometry/vec2.hpp"
#include "gfx/platform/app.hpp"
#include "gfx/platform/window.hpp"
#include "gfx/text/font.hpp"
#include "libcore/result.hpp"
#include "libcore/shared.hpp"


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

    auto t = wgfx::Typeface::from_file("./meta/assets/oswald.ttf").copied();

    auto font = wgfx::Font::load_font(t, 96 * window->dpi()).copied();



    auto sfont = core::SharedPtr<wgfx::Font>::make(font);



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

    float l = 0.f;
    while(true)
    {
        wgfx::Canvas* frame=  window->create_frame();

        //log::log$("ran frame");



        log::log$("green: l:{} - a:{} - b:{}", (long)(wgfx::GREEN.lightness * 100), (long)(wgfx::GREEN.a_green_rediness * 100), (long)(wgfx::GREEN.b_blue_yelowness * 100));

        log::log$("red: {}", wgfx::RED.toRgba8());

        log::log$("blue: {}", wgfx::BLUE.toRgba8());

        log::log$("green: {}", wgfx::GREEN.toRgba8());



        l += 0.5;
        wgfx::CompositeColor color = wgfx::CompositeColor::fromOklch(70.4f/100.f, 0.295, l);


        frame->clear(wgfx::SLATE_WHITE);

        //frame->drawRect(wgfx::GRect(x, y, 256, 256), wgfx::CompositeColor::fromOklch((float)63.7/100, 0, 0));


      //  frame->drawContour(font.shapes['@'].gfx_contour, wgfx::CompositeColor::fromOklch(63.7/100, 0, 0), wgfx::Vec2(x,y));
      //

        frame->drawText(wgfx::Vec2(200.f * window->dpi(),150.f * window->dpi()), "@Hello World ", sfont, wgfx::RED);


        frame->drawText(wgfx::Vec2(200.f * window->dpi(),250.f * window->dpi()), "@Hello World ", sfont, wgfx::GREEN);

        frame->drawText(wgfx::Vec2(200.f * window->dpi(),350.f * window->dpi()), "@Hello World ", sfont, wgfx::BLUE);

        frame->drawText(wgfx::Vec2(200.f * window->dpi(),450.f * window->dpi()), "@Hello World ", sfont, wgfx::CYAN);

        frame->drawText(wgfx::Vec2(200.f * window->dpi(),550.f * window->dpi()), "@Hello World ", sfont, wgfx::YELLOW);

        frame->drawText(wgfx::Vec2(200.f * window->dpi(),650.f * window->dpi()), "@Hello World ", sfont, color);

        frame->drawRect(
            wgfx::GRect::from_size(800.f * window->dpi(), 300.f * window->dpi(), 256.f * window->dpi(), 256.f * window->dpi()),
            wgfx::Painter::stroked(wgfx::FUCHSIA, 10.f),
            (cosf(l*0.001f) +1.f)/2.f);

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
