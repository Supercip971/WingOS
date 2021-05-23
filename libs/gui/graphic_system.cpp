#include <gui/blur.h>
#include <gui/font/basic_8x8_font.h>
#include <gui/graphic_system.h>
#include <kern/kernel_util.h>
#include <kern/mem_util.h>
#include <kern/syscall.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
namespace gui
{

    void graphic_context::send_data(gui::graphic_system_service_protocol *protocol)
    {
        connection.send(protocol, sizeof(gui::graphic_system_service_protocol));
    }
    uint64_t graphic_context::send_data_with_result(gui::graphic_system_service_protocol *protocol)
    {

        connection.send(protocol, sizeof(gui::graphic_system_service_protocol));
        while (true)
        {
            graphic_system_return_info ret;
            size_t readed = connection.wait_receive(&ret, sizeof(graphic_system_return_info));
            if (ret.checksum != gui::GRAPHIC_SYSTEM_INFO_SEND::RETURN_RESULT)
            { // if it is not a return result, use a update_input,
                cache_input_info();
            }
            else
            {
                return ret.return_val;
            }
        }
    }
    void graphic_context::update_info_from_input_info(graphic_system_update_info info)
    {
        switch (info.info_type)
        {
        case gui::GRAPHIC_SYSTEM_INFO_SEND::WINDOW_ATTRIBUTE_CHANGED:
        {
            this->focused = info.info_window_attribute_changed_info.focused;
            printf("focused new state: %i \n", info.info_window_attribute_changed_info.focused);
            this->context_width = info.info_window_attribute_changed_info.new_width;
            this->context_height = info.info_window_attribute_changed_info.new_height;
            break;
        }
        default:
        {
            break;
        }
        }
    }
    bool graphic_context::cache_input_info()
    {
        graphic_system_update_info info;
        size_t readed = connection.receive(&info, sizeof(graphic_system_update_info));
        if (readed == sizeof(graphic_system_update_info))
        {

            cached_received_data.push_front(info);
            return true;
        }
        return false;
    }
    graphic_system_update_info graphic_context::update_input_info()
    {
        if (cached_received_data.size() != 0)
        { // the cached table contain something
            graphic_system_update_info info = cached_received_data[0];
            cached_received_data.remove(0);
            update_info_from_input_info(info);
            return info;
        }
        else
        {
            // if the cached table was clear but when we try to find a new one it is not, then we retry
            if (cache_input_info() != false)
            {
                return update_input_info();
            }
            else
            {
                graphic_system_update_info info = {0};
                return info;
            }
        }
    }
    graphic_context::graphic_context(uint64_t width, uint64_t height, const char *name) : connection("graphic_service.ipc")
    {
        connection.wait_accepted();
        focused = false;
        context_height = height;
        context_width = width;
        size_t name_length = strlen(name) + 1;
        context_name = (char *)malloc(name_length);
        for (size_t i = 0; i <= name_length; i++)
        {
            context_name[i] = name[i];
        }

        gui::graphic_system_service_protocol tolaunch = {0};
        tolaunch.request_type = gui::GRAPHIC_SYSTEM_REQUEST::CREATE_WINDOW;
        tolaunch.create_window_info.height = height;
        tolaunch.create_window_info.width = width;
        wid = send_data_with_result(&tolaunch);

        gui::graphic_system_service_protocol get_bbuffer = {0};
        get_bbuffer.request_type = gui::GRAPHIC_SYSTEM_REQUEST::GET_WINDOW_BACK_BUFFER;
        get_bbuffer.get_request.window_handler_code = wid;

        back_buffer = (gui::color *)send_data_with_result(&get_bbuffer);
        ;
    }

    void graphic_context::clear_buffer(const color color)
    {
        const uint64_t context_length = (context_width * context_height);
        raw_clear_buffer(back_buffer, context_length, color);
    }

    void swap_buffers(gui::color *buffer1, const gui::color *buffer2, uint64_t buffer_length)
    {
        uint64_t buffer_length_r64 = buffer_length / 2;
        uint64_t *to = (uint64_t *)buffer1;
        const uint64_t *from = (const uint64_t *)buffer2;
        for (size_t i = 0; i < buffer_length_r64; i++)
        {
            to[i] = from[i];
        }
    }
    void graphic_context::swap_buffer()
    {

        gui::graphic_system_service_protocol swap_request = {0};
        swap_request.request_type = gui::GRAPHIC_SYSTEM_REQUEST::SWAP_WINDOW_BUFFER;
        swap_request.get_request.window_handler_code = wid;
        send_data_with_result(&swap_request);
    }

    void raw_clear_buffer(gui::color *buffer, uint64_t size, gui::color value)
    {
        const uint64_t msize = size / 2; // copy uint64_t
        uint64_t *conv_buffer = (uint64_t *)buffer;
        const uint64_t v = (uint64_t)value.pix | ((uint64_t)value.pix << 32);
        for (size_t i = 0; i < msize; i++)
        {
            conv_buffer[i] = v;
        }
    }

    void graphic_context::draw_rectangle(const uint64_t x, const uint64_t y, const uint64_t width, const uint64_t height, const color col)
    {
        const uint64_t this_width = this->context_width;
        const uint64_t limit = context_height * context_width;
        for (size_t rx = 0; rx < width; rx++)
        {
            for (size_t ry = 0; ry < height; ry++)
            {
                const uint64_t pos_f = (x + rx) + (y + ry) * this_width;
                if (pos_f >= limit)
                {
                    return;
                }
                if (col.a != 255)
                {

                    back_buffer[pos_f].blend(col);
                }
                else
                {

                    back_buffer[pos_f] = col;
                }
            }
        }
    }

    void graphic_context::draw_basic_char(const uint64_t x, const uint64_t y, const uint8_t chr, const color color)
    {
        for (size_t cx = 0; cx < 8; cx++)
        {
            for (size_t cy = 0; cy < 8; cy++)
            {
                if (((font8x8_basic[chr][cy] >> cx) & 1) == true)
                {
                    back_buffer[(cx + x) + (cy + y) * context_width] = color;
                }
            }
        }
    }

    void graphic_context::draw_basic_string(const uint64_t x, const uint64_t y, const char *str, const color color)
    {
        const uint64_t str_length = strlen(str);
        uint64_t cur_x = x;
        uint64_t cur_y = y;
        for (uint64_t i = 0; i < str_length; i++)
        {
            if (str[i] == '\n')
            {
                cur_x = x;
                cur_y += 8;
                continue;
            }
            draw_basic_char(cur_x, cur_y, str[i] & 0x7F, color);
            cur_x += 8;
        }
    }

    sys::raw_pos graphic_context::get_graphic_context_position()
    {
        gui::graphic_system_service_protocol request = {0};
        request.request_type = gui::GRAPHIC_SYSTEM_REQUEST::GET_WINDOW_POSITION;
        request.get_request.window_handler_code = wid;
        uint64_t v = send_data_with_result(&request);
        sys::raw_pos p;
        p.pos = v;
        return p;
    }
    void graphic_context::resize(uint64_t new_width, uint64_t new_height)
    {
        gui::graphic_system_service_protocol request = {0};
        request.request_type = gui::GRAPHIC_SYSTEM_REQUEST::WINDOW_RESIZE;
        request.resize_request.window_handler_code = wid;
        request.resize_request.new_size.rpos.x = new_width;
        request.resize_request.new_size.rpos.y = new_width;

        uint64_t v = send_data_with_result(&request);

        back_buffer = (gui::color*)v;

    }

    void graphic_context::set_graphic_context_position(const sys::raw_pos position)
    {
        gui::graphic_system_service_protocol request = {0};
        request.request_type = gui::GRAPHIC_SYSTEM_REQUEST::SET_WINDOW_POSITION;
        request.set_pos.window_handler_code = wid;
        request.set_pos.position = position;
        send_data(&request);
    }
    uint64_t get_basic_font_width_text(const char *text)
    {
        uint64_t clen = strlen(text);
        return clen * 8;
    }
    void graphic_context::apply_blur(uint64_t fromx, uint64_t fromy, uint64_t width, uint64_t height)
    {
        stackblur((uint8_t *)back_buffer, context_width, context_height, 10, fromx, fromx + width, fromy, fromy + height);
    }
    void graphic_context::draw_filled_circle_part(const pos origin, const int radius, const color color, const filled_circle_part part)
    {
        const int radius_to_ckeck = radius * radius;
        int y_start = 0;

        if (part == filled_circle_part::BOTTOM_LEFT || part == filled_circle_part::BOTTOM_RIGHT)
        {
            y_start = 0;
        }
        else
        {
            y_start = -radius;
        }

        int y_max = 0;

        if (part == filled_circle_part::BOTTOM_LEFT || part == filled_circle_part::BOTTOM_RIGHT)
        {
            y_max = radius;
        }
        else
        {
            y_max = 0;
        }

        int x_start = 0;
        if (part == filled_circle_part::BOTTOM_LEFT || part == filled_circle_part::TOP_LEFT)
        {
            x_start = -radius;
        }
        else
        {
            x_start = 0;
        }

        int x_max = 0;
        if (part == filled_circle_part::BOTTOM_LEFT || part == filled_circle_part::TOP_LEFT)
        {
            x_max = 0;
        }
        else
        {
            x_max = radius;
        }
        if (origin.x + x_start < 0)
        {
            x_start = origin.x - x_start;
        }
        if (origin.y + y_start < 0)
        {
            y_start = origin.y - y_start;
        }
        for (int y = y_start; y < y_max; y++)
        {

            const int y_to_check = y * y;
            const int y_end_pos = (origin.y + y) * context_width;
            for (int x = x_start; x < x_max; x++)
            {

                const int x_to_check = x * x;
                if (y_to_check + x_to_check < radius_to_ckeck)
                {
                    if (color.a != 255)
                    {

                        back_buffer[origin.x + x + y_end_pos].blend(color);
                    }
                    else
                    {

                        back_buffer[origin.x + x + y_end_pos].pix = color.pix;
                    }
                }
            }
        }
    }
    void graphic_context::draw_filled_circle(const pos origin, const int radius, const color color)
    {
        const int radius_to_ckeck = radius * radius;
        for (int y = -radius; y <= radius; y++)
        {

            const int y_to_check = y * y;
            const int y_end_pos = (origin.y + y) * context_width;
            for (int x = -radius; x <= radius; x++)
            {

                const int x_to_check = x * x;
                if (y_to_check + x_to_check < radius_to_ckeck)
                {
                    if (color.a != 255)
                    {

                        back_buffer[origin.x + x + y_end_pos].blend(color);
                    }
                    else
                    {

                        back_buffer[origin.x + x + y_end_pos] = color;
                    }
                }
            }
        }
    }

    void graphic_context::draw_rounded_rectangle(size_t radius, const uint64_t x, const uint64_t y, const uint64_t width, const uint64_t height, const color color)
    {
        if (radius > height / 2)
        {
            radius = height / 2;
        }
        if (radius > width / 2)
        {
            radius = width;
        }

        draw_rectangle(x + radius, y, width - (radius * 2), height, color);
        draw_rectangle(x, y + radius, width, height - (radius * 2), color);
        const int left_x = x + radius;
        const int top_y = y + radius;
        const int right_x = x + width - radius - 1;
        const int bottom_y = y + height - radius - 1;
        draw_filled_circle_part(pos(left_x, top_y), radius, color, TOP_LEFT);         // top left
        draw_filled_circle_part(pos(right_x, top_y), radius, color, TOP_RIGHT);       // top right
        draw_filled_circle_part(pos(left_x, bottom_y), radius, color, BOTTOM_LEFT);   // bottom left
        draw_filled_circle_part(pos(right_x, bottom_y), radius, color, BOTTOM_RIGHT); // bottom right
    }
    void graphic_context::draw_rounded_rectangle_b(size_t radius, const uint64_t x, const uint64_t y, const uint64_t width, const uint64_t height, const color color)
    {

        draw_rectangle(x, y, width, height, gui::color(0, 0, 0, 100));
        apply_blur(x - 10, y - 10, width + 20, height + 20);
        draw_rounded_rectangle(radius, x, y, width, height, color);
    }

    void graphic_context::set_on_top()
    {
        graphic_system_service_protocol request = {0};
        request.request_type = GRAPHIC_SYSTEM_REQUEST::WINDOW_DEPTH_ACTION;
        request.depth_request.window_handler_code = wid;
        request.depth_request.set = true;
        request.depth_request.type = gui::ON_TOP;

        send_data(&request);
        connection.send(&request, sizeof(gui::graphic_system_service_protocol));
    }
    void graphic_context::set_as_background()
    {
        graphic_system_service_protocol request = {0};
        request.request_type = GRAPHIC_SYSTEM_REQUEST::WINDOW_DEPTH_ACTION;
        request.depth_request.window_handler_code = wid;
        request.depth_request.set = true;
        request.depth_request.type = gui::BACKGROUND;
        send_data(&request);
    }
    void graphic_context::set_on_top_of_background()
    {
        graphic_system_service_protocol request = {0};
        request.request_type = GRAPHIC_SYSTEM_REQUEST::WINDOW_DEPTH_ACTION;
        request.depth_request.window_handler_code = wid;
        request.depth_request.set = true;
        request.depth_request.type = TOP_BACKGROUND;
        send_data(&request);
    }
    bool graphic_context::is_on_top()
    {
        return focused;
    }
    bool graphic_context::is_mouse_inside()
    {

        uint64_t result = sys::sys$get_process_global_data(0, "initfs/graphic_service.exe");
        if (result == wid)
        {
            return true;
        }
        return false;
    }

} // namespace gui
