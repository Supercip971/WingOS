#include <libcore/fmt/log.hpp>

#include "libcore/fmt/fmt_str.hpp"
#include "libcore/str_writer.hpp"

#include "gfx/backend.hpp"
#include "gfx/canvas/canvas.hpp"
#include "gfx/color.hpp"
#include "gfx/event/event.hpp"
#include "gfx/platform/app.hpp"
#include "gfx/platform/window.hpp"
#include "libcore/fmt/flags.hpp"
#include "libcore/result.hpp"
#include "libcore/shared.hpp"
#include "ui/context.hpp"
#include "ui/font-manager.hpp"
#include "ui/image-manager.hpp"
#include "ui/widgets/button.hpp"
#include "ui/widgets/callback.hpp"
#include "ui/widgets/centered.hpp"
#include "ui/widgets/drageable.hpp"
#include "ui/widgets/image.hpp"
#include "ui/widgets/padded.hpp"
#include "ui/widgets/root.hpp"
#include "ui/widgets/stacked.hpp"
#include "ui/widgets/statefull.hpp"
#include "ui/widgets/text.hpp"
#include "ui/widgets/vflex.hpp"
#include "ui/widgets/widget.hpp"
#include "ui/widgets/window.hpp"

struct MyState
{
    int counter;
};

class CustomWidget2 : public fc::Statefull<MyState>
{

public:
    fc::SharedPtr<fc::Widget> build(const fc::UiContext &ctx) override
    {
        auto res = fmt::format_str("Counter 2: {}", counter | fmt::FMT_PAD_ZERO);

        return $<fc::VFlex>(

            $<fc::TextWidget>((res.take()),
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
                $<fc::LPadded>(fc::Padded().horizontal(16 * ctx.dpi).down(10.f), $<fc::TextWidget>("hello world!",
                                                                                                   fc::FontsRepo::the().find("oswald@96")))));
    }
};

;

class WingosWidget : public fc::Widget
{

public:
    fc::SharedPtr<fc::Widget> build(const fc::UiContext &ctx) override
    {
        auto res = fmt::format_str("Wingos - An open source microkernel based operating system");

        return $<fc::VFlex>(
            $<fc::LPadded>(fc::Padded().horizontal(16 * ctx.dpi).vertical(16.f * ctx.dpi),
                           $<fc::TextWidget>(res.take(),
                                             fc::FontsRepo::the().find("oswald@32"))),
            $<fc::LPadded>(fc::Padded().horizontal(16 * ctx.dpi).vertical(16.f * ctx.dpi),

                           $<fc::ImageWidget>(
                               fc::TextureRepo::the().find("logo-dark-low")))

        );
    }
};

class CustomWidget : public fc::Statefull<MyState>
{

public:
    fc::SharedPtr<fc::Widget> build(const fc::UiContext &ctx) override
    {
        (void)ctx;

        return $<fc::_Root>(wgfx::BLUE,

                            $<fc::Stacked>(
                                $<fc::ImageWidget>(
                                    fc::TextureRepo::the().find("liquid-blue")),
                                $<fc::WindowWidget>(

                                    fc::WindowWidgetParams(900, 500).title(
                                        "button window"),

                                    $<fc::Centered>($<CustomWidget2>())),
                                $<fc::WindowWidget>(

                                    fc::WindowWidgetParams(900, 500).title(
                                        "about"),

                                    ($<WingosWidget>()))));
    }
};

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    fmt::log$("Hello, World!");

    wgfx::initialize_platform();

    auto window = wgfx::PlatformWindow::create_native(wgfx::BackendsKinds::BACKEND_KIND_RASTER).take();

    window->attach();

    fc::TextureRepo::the().load(fc::WStr::copy("liquid-blue"), "/meta/assets/pawel-czerwinski-blue-liquid-halfres.png");
    fc::TextureRepo::the().load(fc::WStr::copy("logo-dark-low"), "/meta/assets/logo-dark-low.png");

    fc::FontsRepo::the().load(fc::WStr::copy("oswald@96"), "/meta/assets/oswald.ttf", 96 * window->dpi());

    fc::FontsRepo::the().load(fc::WStr::copy("oswald@32"), "/meta/assets/oswald.ttf", 32 * window->dpi());

    auto vwidgt = fc::SharedPtr<CustomWidget>::make().static_pointer_cast<fc::Widget>();

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
                vwidgt->distributeEvent(ev);
            }
        } while (ev.kind != wgfx::UEvent::Kind::NONE);

        wgfx::Canvas *frame = window->create_frame();

        vwidgt->update_dirty(ctx);

        vwidgt->update_layout(ctx, wgfx::GRect(0, 0, window->width(), window->height()));

        vwidgt->render_dirty(ctx, *frame);

        window->end_frame(frame);
    }
}
