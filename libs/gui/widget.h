#pragma once
#include <gui/graphic_system.h>
#include <stddef.h>
#include <utils/container/wvector.h>

namespace gui
{
    class widget
    {
    protected:
        int64_t widget_width;
        int64_t widget_height;
        int64_t widget_x;
        int64_t widget_y;
        bool widget_should_draw = true;
        bool is_position_inside_widget(const pos pos);
        bool is_forced_size = false;

    public:
        bool is_manually_sized()
        {
            return is_forced_size;
        }

        long width() { return widget_width; };
        long height() { return widget_height; };
        long x() { return widget_x; };
        long y() { return widget_y; };
        widget();
        virtual void update_widget(){};
        virtual void callback(graphic_system_update_info &info){};

        virtual void resize(int64_t new_x, int64_t new_y, int64_t new_width, int64_t new_height)
        {
            widget_x = new_x;
            widget_y = new_y;
            widget_width = new_width;
            widget_height = new_height;
            update_widget();
        }
        virtual void draw_widget(graphic_context &context) = 0;
        virtual bool should_redraw()
        {
            return widget_should_draw;
        }
        virtual bool set_should_redraw(bool value)
        {
            return widget_should_draw;
        }
        // note: i do void* as c++ doesn't like recursive include
        virtual void init_widget(void *new_parent){};
    };

    enum layout_type
    {
        LAYOUT_VERTICAL = 0,
        LAYOUT_HORIZONTAL = 1,
    };

    class widget_container : public widget

    {

        utils::vector<widget *> list;
        int layout_type;
        size_t _padding;
        void *parent;
        void relayout();

    public:

        void set_padding(size_t amount)
        {
            _padding = amount;
        }
        
        size_t padding() const
        {
            return _padding;
        }

        widget_container() : widget()
        {
            list.clear();
            set_padding(4);
            layout_type = LAYOUT_VERTICAL;
        }
        virtual void update_widget() override
        {
            for (size_t i = 0; i < list.size(); i++)
            {
                (*list[i]).update_widget();
            }
        }
        virtual void callback(graphic_system_update_info &info) override
        {
            for (size_t i = 0; i < list.size(); i++)
            {
                (*list[i]).callback(info);
            }
        }
        virtual void draw_widget(graphic_context &context) override;
        virtual void init_widget(void *new_parent) override;
        virtual void resize(int64_t new_x, int64_t new_y, int64_t new_width, int64_t new_height)
        {
            widget_x = new_x;
            widget_y = new_y;
            widget_width = new_width;
            widget_height = new_height;
            relayout();
            update_widget();
        }
        void add_widget(widget *widget)
        {
            widget->init_widget(parent);
            list.push_back(widget);
            relayout();
        }
        void callback_update(graphic_system_update_info &info)
        {
            for (size_t i = 0; i < list.size(); i++)
            {
                (*list[i]).callback(info);
            }
        }
        virtual bool should_redraw()
        {
            for (size_t i = 0; i < list.size(); i++)
            {
                if ((*list[i]).should_redraw())
                {
                    return true;
                }
            }
            return false;
        }
    };

} // namespace gui
