#pragma once

#include "gfx/canvas/cmd.hpp"
#include "gfx/canvas/draw_context.hpp"
#include "gfx/color.hpp"
#include "gfx/geometry/rect.hpp"
#include "gfx/text/utf-text.hpp"
#include "libcore/ds/vec.hpp"
#include "libcore/shared.hpp"
namespace wgfx
{




using RenderCommands = core::Vec<RenderCommand>;

class Canvas
{
protected:
    RenderCommands commands;



public:

    wgfx::GRect size;
    virtual ~Canvas() {}


    Canvas record()
    {
        Canvas canvas = {};
        return canvas;
    }

    RenderCommands& stopRecord() {
        return commands;
    }

    void recordApply(RenderCommands const & cmds, wgfx::GRect constraint)
    {
        for (auto &cmd : cmds)
        {
            auto cmd_copy = cmd;
            (void)constraint;

            commands.push(cmd);
        }
    }

    void clear(CompositeColor color)
    {
        RenderCommand cmd = RenderCommand::from((FillCommand){
            .paint = color,
        });
        cmd.constraint = size;
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

    void drawText(Vec2 start, Utf8Str string, core::SharedPtr<Font> const &font, CompositeColor color)
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
