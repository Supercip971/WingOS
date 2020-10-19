#pragma once
#include <klib/graphic_system.h>
#include <stddef.h>

namespace gui
{
    class widget
    {
    protected:
        uint64_t widget_width;
        uint64_t widget_height;
        uint64_t widget_x;
        uint64_t widget_y;

        bool widget_should_draw = false;
        bool is_position_inside_widget(const sys::pos pos);

    public:
        widget();
        virtual void update_widget() = 0;
        virtual void draw_widget(sys::graphic_context &context) = 0;
        // note: i do void* as c++ doesn't like recursive include
        virtual void init_widget(void *new_parent) = 0;
    };

    class widget_list
    {

        widget **list;
        uint64_t list_length = 0;

    public:
        void init(size_t length);

        void add_widget(widget *widget);

        void update_all();
        void draw_all(sys::graphic_context &context);
    };
} // namespace gui
