#pragma once
#include <stdint.h>
#include <klib/graphic_system.h>
namespace gui {
    class window{
        const char* window_name;
        sys::graphic_context graphic_context;
        uint64_t width;
        uint64_t height;
        uint64_t current_tick;
        // widget* widget_list; for later ;)
    public:
        // create a simple window
        // if you don't want a update_function give just a nullptr,
        // it will just skip the update
        // todo: add a after_draw_update() and a before_draw_update()
        window(const char* name, uint64_t window_width, uint64_t window_height);
        uint64_t start();
    };
}
