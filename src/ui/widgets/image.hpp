#pragma once

#include "libcore/str_writer.hpp"

#include "gfx/color.hpp"
#include "gfx/geometry/rect.hpp"
#include "gfx/geometry/vec2.hpp"
#include "gfx/image/image.hpp"
#include "gfx/image/texture.hpp"
#include "gfx/text/font.hpp"
#include "libcore/shared.hpp"
#include "widget.hpp"

namespace fc
{

struct ImageWidgetLayout
{
    bool _keep_ratio = true;
    bool _filled = true;

    ImageWidgetLayout() = default;

    ImageWidgetLayout filling(bool filled)
    {
        _filled = filled;
        return *this;
    }
    ImageWidgetLayout keep_ratio(bool keep_ratio)
    {
        _keep_ratio = keep_ratio;
        return *this;
    }
};

class ImageWidget : public Widget
{

    core::SharedPtr<wgfx::Texture> img;
    ImageWidgetLayout _layout = ImageWidgetLayout();

public:
    ~ImageWidget() override = default;
    ImageWidget(core::SharedPtr<wgfx::Texture> &_img) : img(_img) {}

    ImageWidget(core::SharedPtr<wgfx::Texture> _img) : img(_img) {}

    ImageWidget(core::SharedPtr<wgfx::Texture> _img, ImageWidgetLayout layout) : img(_img), _layout(layout) {}

    wgfx::Vec2 preferred_size(wgfx::Vec2 constraint) const override
    {

        (void)constraint;

        return constraint;

        // v.y = core::max(v.y, constraint.y);
    }

    void render(UiContext const &ctx, wgfx::Canvas &canvas) const override
    {
        //      fmt::log$("renderign text at: {}-{} {}", (long)bounds().start.x, (long)bounds().start.y, val.view());
        (void)ctx;
        //     canvas.drawText(bounds().start + wgfx::Vec2(0.f, 96.f), val.view(), font, wgfx::CONTAINER_TEXT);

        if (_layout._keep_ratio)
        {

            if (_layout._filled)
            {
                float ratio = (float)img->image()->width() / (float)img->image()->height();

                auto vbounds = bounds();

                // height > width

                if (ratio > 1.0f)
                {
                    vbounds.width(vbounds.height() * ratio);
                }
                else
                {
                    vbounds.height(vbounds.width() / ratio);
                }

                if (ratio > 1.0f)
                {
                    vbounds = vbounds.with_size(vbounds.size() * bounds().width() / vbounds.width());
                }
                else
                {
                    vbounds = vbounds.with_size(vbounds.size() * bounds().height() / vbounds.height());
                }

                canvas.drawImage(img, vbounds);
            }
            else
            {

                float ratio = (float)img->image()->width() / (float)img->image()->height();

                auto vbounds = bounds();
                if (ratio > 1.0f)
                {

                    vbounds.width(vbounds.height() * ratio);
                }
                else
                {
                    vbounds.height(vbounds.width() / ratio);
                }
                canvas.drawImage(img, vbounds);
            }
        }
        else
            canvas.drawImage(img, bounds());
    }
};
} // namespace fc
