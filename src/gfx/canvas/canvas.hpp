#pragma once

#include "gfx/canvas/draw_context.hpp"

#include "gfx/canvas/cmd.hpp"
#include "gfx/color.hpp"
#include "gfx/text/utf-text.hpp"
#include "libcore/ds/vec.hpp"
#include "libcore/shared.hpp"
namespace wgfx
{

class Canvas
{
protected:
    core::Vec<RenderCommand> commands;

public:
    virtual ~Canvas() {}

    void clear(CompositeColor color)
    {
        RenderCommand cmd = RenderCommand::from((FillCommand){
            .paint = color,
        });
        commands.clear();
        commands.push(cmd);
    }


    void drawRect(wgfx::GRect rect, Painter paint, float radius = 0.0f)
    {

        RenderCommand cmd = RenderCommand::from((RectCommand){
            .paint = paint,
            .rect = rect,
            .radius = radius,
        });
        commands.push(cmd);
    }

    void drawText(Vec2 start, Utf8Str string, core::SharedPtr<Font> &font, CompositeColor color)
    {
        RenderCommand cmd = RenderCommand::from((TextCommand){
            .paint = color,
            .str = string,
            .font = font,
            .pos = start,
        });

        commands.push(cmd);
    }


    void drawContour(core::SharedPtr<Contour> & contour, CompositeColor color, Vec2 pos)
    {
        RenderCommand cmd = RenderCommand::from((ContourCommand){
            .paint = color,
            .contour = contour,
            .pos = pos,
        });
        commands.push(cmd);
    }


    virtual void apply(DrawContext const &ctx, RenderCommand const &cmd)
    {
        (void)ctx;
        (void)cmd;
    };
    virtual void flush()
    {
        DrawContext ctx = {};
        for (auto &cmd : commands)
        {
            apply(ctx, cmd);
        }
        commands.clear();
    };
};
} // namespace wgfx
