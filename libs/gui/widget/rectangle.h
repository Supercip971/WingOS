#pragma once
#include <gui/widget.h>

namespace gui
{

    class rectangle_widget : public widget
    {
        color col;

    public:
        rectangle_widget();
        rectangle_widget(uint64_t x, uint64_t y, uint64_t width, uint64_t heigth, color color);

        virtual void update_widget() override;
        virtual void draw_widget(graphic_context &context) override;
        virtual void init_widget(void *new_parent) override;
    };
} // namespace gui
