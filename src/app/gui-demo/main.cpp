#include <libcore/fmt/log.hpp>

#include "libcore/fmt/fmt_str.hpp"
#include "libcore/str_writer.hpp"

#include "gfx/backend.hpp"
#include "gfx/canvas/cmd.hpp"
#include "gfx/color.hpp"
#include "gfx/event/event.hpp"
#include "gfx/geometry/vec2.hpp"
#include "gfx/platform/app.hpp"
#include "gfx/platform/window.hpp"
#include "gfx/text/font.hpp"
#include "libcore/fmt/flags.hpp"
#include "libcore/result.hpp"
#include "libcore/shared.hpp"
#include "ui/font-manager.hpp"
#include "ui/image-manager.hpp"
#include "ui/widgets/button.hpp"
#include "ui/widgets/callback.hpp"
#include "ui/widgets/centered.hpp"
#include "ui/widgets/container.hpp"
#include "ui/widgets/image.hpp"
#include "ui/widgets/padded.hpp"
#include "ui/widgets/root.hpp"
#include "ui/widgets/sized.hpp"
#include "ui/widgets/stacked.hpp"
#include "ui/widgets/statefull.hpp"
#include "ui/widgets/text.hpp"
#include "ui/widgets/vflex.hpp"
#include "ui/widgets/widget.hpp"

struct MyState
{

    int counter;
};
class CustomWidget2 : public fc::Statefull<MyState>
{

public:
    core::SharedPtr<fc::Widget> build(const fc::UiContext & ctx) override
    {

        auto res = fmt::format_str("Counter 2: {}", counter | fmt::FMT_PAD_ZERO);

        return $<fc::VFlex>(

            $<fc::TextWidget>(core::move(res.take()),
                              fc::FontsRepo::the().find("oswald@96")),
            $<fc::Button>(

                fc::ButtonParams()
                    .bg(
                        wgfx::CompositeColor::fromOklch(0.7245, 0.1239, 156.12))
                    .border(
                        wgfx::CompositeColor::fromOklch(0.5937, 0.0999, 156.09))
                    .shadowy(
                        wgfx::CompositeColor::fromOklch(0.6554, 0.114, 156.2)),

                fc::AutoCallback$([](CustomWidget2 *w2)
                                  { w2->setState([&]()
                                                { w2->counter++; }); }),
                $<fc::LPadded>(fc::Padded().horizontal(16 * ctx.dpi).down(100.f), $<fc::TextWidget>("hello world! j  @g",
                                                                              fc::FontsRepo::the().find("oswald@96")))));
    }
};
class CustomWidget : public fc::Statefull<MyState>
{

public:
    core::SharedPtr<fc::Widget> build(const fc::UiContext & ctx) override
    {

        return $<fc::_Root>(wgfx::BLUE,

                            $<fc::Stacked>(
                                $<fc::ImageWidget>(
                                    fc::TextureRepo::the().find("liquid-blue")),
                                $<fc::Centered>(
                                    $<fc::Sized>(
                                        fc::LayoutSize(300.f * ctx.dpi, 300.f * ctx.dpi).min_width(200 * ctx.dpi).max_width(800 * ctx.dpi),
                                        $<fc::Container>(
                                            fc::ContainerParms().bg(wgfx::CONTAINER_FILL.transparentize(0.3)).border(wgfx::CONTAINER_BORDER, 1).radius(8),
                                            $<fc::Centered>($<CustomWidget2>()))))));
    }
};
int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    fmt::log$("Hello, World!");

    wgfx::initialize_platform();

    auto window = wgfx::PlatformWindow::create_native(wgfx::BackendsKinds::BACKEND_KIND_RASTER).copied();

    window->attach();

    fc::TextureRepo::the().load(core::WStr::copy("liquid-blue"), "./meta/assets/pawel-czerwinski-blue-liquid-halfres.png");
    fc::FontsRepo::the().load(core::WStr::copy("oswald@96"), "./meta/assets/oswald.ttf", 96 * window->dpi());

    float l = 0.f;

    auto vwidgt = core::SharedPtr<CustomWidget>::make().static_pointer_cast<fc::Widget>();

    fc::UiContext ctx = {};
    ctx.theme = fc::Theme::wingos();

    ctx.dpi = window->dpi();
//    ctx.enable_debug_layout = true;
    vwidgt->mount(ctx);
    // vwidgt->build({});
    //

    vwidgt->relayout(ctx, wgfx::GRect(0, 0, window->width(), window->height()));


    vwidgt->update_dirty(ctx);


    vwidgt->update_layout(ctx, wgfx::GRect(0, 0, window->width(), window->height()));


    while (true)
    {

        wgfx::UEvent ev = {};

        do
        {
            ev = window->query_event();

            if (ev.kind != wgfx::UEvent::Kind::NONE)
            {
                fmt::log$("Event: {} at ({}, {})", (long)ev.kind, (long)ev.at.x, (long)ev.at.y);
                vwidgt->acquireEvent(ev);
            }
        } while (ev.kind != wgfx::UEvent::Kind::NONE);


        wgfx::Canvas *frame = window->create_frame();

        vwidgt->update_dirty(ctx);

        vwidgt->update_layout(ctx, wgfx::GRect(0, 0, window->width(), window->height()));

        //   fmt::log$("green: l:{} - a:{} - b:{}", (long)(wgfx::GREEN.lightness * 100), (long)(wgfx::GREEN.a_green_rediness * 100), (long)(wgfx::GREEN.b_blue_yelowness * 100));

        //   fmt::log$("red: {}", wgfx::RED.toRgba8());

        //   fmt::log$("blue: {}", wgfx::BLUE.toRgba8());

        //   fmt::log$("green: {}", wgfx::GREEN.toRgba8());

        l += 0.5;
        (void)l;
        // wgfx::CompositeColor color = wgfx::CompositeColor::fromOklch(70.4f/100.f, 0.295, l);

        vwidgt->render_dirty(ctx, *frame);


        // vwidgt->dump();
        // frame->drawRect(wgfx::GRect(x, y, 256, 256), wgfx::CompositeColor::fromOklch((float)63.7/100, 0, 0));

        //  frame->drawContour(font.shapes['@'].gfx_contour, wgfx::CompositeColor::fromOklch(63.7/100, 0, 0), wgfx::Vec2(x,y));
        //

        // frame->drawText(wgfx::Vec2(200.f * window->dpi(),150.f * window->dpi()), "@Hello World ", sfont, wgfx::RED);

        // frame->drawText(wgfx::Vec2(200.f * window->dpi(),250.f * window->dpi()), "@Hello World ", sfont, wgfx::GREEN);

        // frame->drawText(wgfx::Vec2(200.f * window->dpi(),350.f * window->dpi()), "@Hello World ", sfont, wgfx::BLUE);

        // frame->drawText(wgfx::Vec2(200.f * window->dpi(),450.f * window->dpi()), "@Hello World ", sfont, wgfx::CYAN);

        // frame->drawText(wgfx::Vec2(200.f * window->dpi(),550.f * window->dpi()), "@Hello World ", sfont, wgfx::YELLOW);

        // frame->drawText(wgfx::Vec2(200.f * window->dpi(),650.f * window->dpi()), "@Hello World ", sfont, color);

        // frame->drawRect(
        //      wgfx::GRect::from_size(800.f * window->dpi(), 300.f * window->dpi(), 256.f * window->dpi(), 256.f * window->dpi()),
        //      wgfx::Painter::stroked(wgfx::FUCHSIA, 10.f),
        //      (cosf(l*0.001f) +1.f)/2.f);


        window->end_frame(frame);

    }
}
