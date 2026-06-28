#pragma once

#include "gfx/canvas/draw_context.hpp"

#include "gfx/canvas/cmd.hpp"
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
    wgfx::GRect context_size;
    core::Vec<wgfx::GRect> scissor_stack;

    virtual ~Canvas() {}

    Canvas record()
    {
        Canvas canvas = {};
        return canvas;
    }

    RenderCommands &stopRecord()
    {
        return commands;
    }

    void recordApply(RenderCommands const &cmds, wgfx::GRect constraint)
    {
        if (cmds.len() == 0)
        {
            return;
        }
        startScissor(constraint);
        for (auto &cmd : cmds)
        {
            commands.push(cmd);
        }
        endScissor();
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

    void drawImage(core::SharedPtr<wgfx::Texture> const &texture, wgfx::GRect rect)
    {
        commands.push(RenderCommand::from((TextureCommand){
            .tex = texture,
            .rect = rect,
        }));
    }

    void drawContour(core::SharedPtr<Contour> &contour, CompositeColor color, Vec2 pos)
    {
        RenderCommand cmd = RenderCommand::from((ContourCommand){
            .paint = color,
            .contour = contour,
            .pos = pos,
        });
        commands.push(cmd);
    }

    void startScissor(wgfx::GRect rect)
    {
        RenderCommand cmd = RenderCommand::from((ScissorCommand){
            .rect = rect,
        });
        commands.push(cmd);
    }

    void endScissor()
    {
        RenderCommand cmd = RenderCommand::from((ScissorCommand){
            .rect = {},
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
        auto original_size = size;
        DrawContext ctx = {};
        for (auto &cmd : commands)
        {
            if (cmd.kind == RenderCommandKind::RENDER_KIND_SCISSOR)
            {
                if (cmd.scissor.rect != wgfx::GRect{})
                {
                    scissor_stack.push(size);

                    size = cmd.scissor.rect.intersect(size);
                }
                else
                {
                    size = scissor_stack.pop();
                }
            }
            else
            {
                apply(ctx, cmd);
            }
        }
        commands.clear();
        scissor_stack.clear();
        size = original_size;
    };
};
} // namespace wgfx
