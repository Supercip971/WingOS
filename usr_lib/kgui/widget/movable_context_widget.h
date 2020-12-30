#pragma once
#include <kgui/window.h>
#include <klib/graphic_system.h>
namespace gui
{

    class movable_context_widget : public widget
    {

        gui::window *parent;
        bool dragging = false;

        sys::pos start_dragging_pos;
        sys::pos current_dragging_pos;
        int drag_time = 0;
        bool start_down;

    public:
        movable_context_widget();
        movable_context_widget(uint64_t x, uint64_t y, uint64_t width, uint64_t height, gui::window *target) __attribute__((__target__("no-sse")));

        virtual void update_widget() override;
        virtual void draw_widget(sys::graphic_context &context) override;
        virtual void init_widget(void *new_parent) override;
    };
} // namespace gui
