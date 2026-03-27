#pragma once

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
            RenderCommand cmd = {
                .kind = RenderCommandKind::RENDER_KIND_FILL,
                .fill = (FillCommand){
                    .paint = color,
                }

            };
            commands.clear();
            commands.push(cmd);
        }


        virtual void apply(){};
    };
}
