#pragma once

#include "external/stb/stb_truetype.h"
#include "gfx/canvas/draw_context.hpp"
#include "gfx/canvas/cmd.hpp"
#include "gfx/color.hpp"
#include "libcore/ds/vec.hpp"
namespace wgfx {
    class Canvas {
        protected:
        core::Vec<RenderCommand> commands;



        public:


        virtual ~Canvas() {}



        void clear(CompositeColor color){
            RenderCommand cmd = RenderCommand::from((FillCommand){
                .paint = color,
            });
            commands.clear();
            commands.push(cmd);
        }

        void drawRect(long x, long y, long width, long height, CompositeColor color)
        {

            RenderCommand cmd = RenderCommand::from((RectCommand){
                .paint = color,
                .rect = {x, y, width+x, height+y},
            });
            commands.push(cmd);
        }


        virtual void apply(DrawContext const &ctx,  RenderCommand const& cmd){
            (void)ctx;
            (void)cmd;
        };
        virtual void flush(){
            DrawContext ctx = {};
            for (auto& cmd : commands) {
                apply(ctx, cmd);
            }
            commands.clear();

        };
    };
}
