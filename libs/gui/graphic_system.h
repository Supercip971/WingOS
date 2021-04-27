#pragma once
#include <gui/raw_graphic.h>
#include <stdint.h>
#include <kern/process_message.h>
namespace gui
{
    // FOR GRAPHIC MESSAGE :
    struct create_graphic_window
    {
        char *name;
        uint64_t width;
        uint64_t height;
        bool resizable; // will not be used
    } __attribute__((packed));
    struct individual_request
    {
        uint64_t window_handler_code;
    } __attribute__((packed));
    struct set_window_pos_request
    {
        uint64_t window_handler_code;
        sys::raw_pos position;
    } __attribute__((packed));
    enum WINDOW_DEPTH_TYPE
    {
        BACKGROUND = 1,     // back
        TOP_BACKGROUND = 2, // like the menu bar
        ON_TOP = 3          // in front
    };

    struct window_depth_request
    {
        uint64_t window_handler_code;
        bool set;
        uint8_t type; // 0 on top
    } __attribute__((packed));
    enum GRAPHIC_SYSTEM_REQUEST
    {
        NULL_REQUEST = 0,
        CREATE_WINDOW = 1,
        GET_WINDOW_BACK_BUFFER = 2,
        SWAP_WINDOW_BUFFER = 3,
        GET_WINDOW_POSITION = 4,
        SET_WINDOW_POSITION = 5,
        WINDOW_DEPTH_ACTION = 6,
    };

    // window event are not used for the moment
    enum WINDOW_EVENT
    {
        EVENT_RESIZE = 0,
        EVENT_MOVE = 1,
    };

    struct graphic_system_service_protocol
    {
        uint16_t request_type;

        union
        {
            create_graphic_window create_window_info;
            individual_request get_request;
            set_window_pos_request set_pos;
            window_depth_request depth_request;
        };
    } __attribute__((packed));
    enum GRAPHIC_SYSTEM_INFO_SEND
    {
        NULL_INFO = 0,
        WINDOW_ATTRIBUTE_CHANGED = 1, // size, position, focus
        WINDOW_CLOSED = 2,
        KEY_INPUT = 3,
        MOUSE_INPUT = 4,
        MOUSE_MOVED = 5,


        RETURN_RESULT = 6,

    };
    struct window_attribute_changed_info {
        uint16_t  new_x;
        uint16_t  new_y;
        uint16_t  new_width;
        uint16_t  new_height;
        bool focused;
    }__attribute__((packed));
    struct window_key_input {
        uint8_t  key_char;
        uint8_t  modifier;
    }__attribute__((packed));
    struct mouse_input {
        uint8_t  mouse_key;
    }__attribute__((packed));
    struct mouse_moved { // position relative to window
        size_t new_x;
        size_t new_y;
    }__attribute__((packed));
    struct graphic_system_update_info
    {
        uint8_t  info_type;
        union
        {
            window_attribute_changed_info info_window_attribute_changed_info;
            window_key_input info_window_key_input;
            mouse_input info_mouse_input;
            mouse_moved info_mouse_moved;
            uint8_t raw_dat[64];
        };
    }__attribute__((packed));


    struct graphic_system_return_info
    {
        uint8_t  checksum;
        uint64_t  return_val;
    }__attribute__((packed));
    // FOR OTHER THING
    struct color
    {
        union
        {
            struct
            {
                uint8_t b;
                uint8_t g;
                uint8_t r;
                uint8_t a;
            };
            uint32_t pix;
        };
        constexpr color()
        {
            r = 0;
            g = 0;
            b = 0;
            a = 0;
        };
        constexpr color(uint8_t vr, uint8_t vg, uint8_t vb, uint8_t va = 255)
        {
            r = vr;
            g = vg;
            b = vb;
            a = va;
        }
        constexpr uint32_t raw() const
        {
            return r << 24 | g << 16 | b << 8 | a;
        }
        constexpr void blend(color val)
        {
            r = ((255 - a) * val.a * val.r + a * r) / a;
            g = ((255 - a) * val.a * val.g + a * g) / a;
            b = ((255 - a) * val.a * val.b + a * b) / a;
            a = (255 - a) * val.a + a;
        }
    };
    static_assert(sizeof(color) == sizeof(uint32_t), "sizeof(color) must be the same as a uint32_t");
    struct pos
    {
        int32_t x;
        int32_t y;
        pos()
        {
            x = 0;
            y = 0;
        }
        pos(int32_t vx, int32_t vy)
        {
            x = vx;
            y = vy;
        }
    } __attribute__((packed));

    class graphic_context
    {
        sys::client_connection connection;
        // when we do a send_data_with_result we can receive an/multiple update from the graphic system
        // in that case we store them in this vector and then we read the result from the sended command by "send_data_with_result"
        utils::vector<graphic_system_update_info> cached_received_data;
        uint64_t context_width;
        uint64_t context_height;

        uint64_t wid = 0;
        color *back_buffer;
        char *context_name;
        int graphic_pid = 0;
        static constexpr int filter_width = 3;
        static constexpr int filter_height = 3;
        static constexpr int divisor = 16;
        int filter[9] = {
            1, 2, 1,
            2, 4, 2,
            1, 2, 1};
        void send_data(gui::graphic_system_service_protocol *protocol);
        uint64_t send_data_with_result(gui::graphic_system_service_protocol* protocol);
        bool cache_input_info();
    public:
        graphic_system_update_info update_input_info();
        graphic_context(uint64_t width, uint64_t height, const char *name) __attribute__((__target__("no-sse")));

        void draw_filled_circle(const pos origin, const int radius, const color color);
        constexpr uint64_t get_window_id() const
        {
            return wid;
        }
        enum filled_circle_part
        {
            TOP_RIGHT = 0,
            BOTTOM_RIGHT,
            BOTTOM_LEFT,
            TOP_LEFT
        };
        constexpr void set_pixel(color p, unsigned int x, unsigned int y)
        {

            back_buffer[(x) + (y)*context_width] = p;
        }
        void apply_blur(uint64_t fromx, uint64_t fromy, uint64_t width, uint64_t height);
        void draw_filled_circle_part(const pos origin, const int radius, const color color, const filled_circle_part part);
        void draw_rounded_rectangle(size_t radius, const uint64_t x, const uint64_t y, const uint64_t width, const uint64_t height, const color color);
        void draw_rounded_rectangle_b(size_t radius, const uint64_t x, const uint64_t y, const uint64_t width, const uint64_t height, const color color);
        void draw_rectangle(const uint64_t x, const uint64_t y, const uint64_t width, const uint64_t height, const color color);
        void draw_basic_char(const uint64_t x, const uint64_t y, const uint8_t chr, const color color);
        void draw_basic_string(const uint64_t x, const uint64_t y, const char *str, const color color);
        void clear_buffer(const color color);
        void swap_buffer();
        void set_on_top();
        void set_as_background();
        void set_on_top_of_background();
        bool is_on_top();
        bool is_mouse_inside();
        sys::raw_pos get_graphic_context_position();
        void set_graphic_context_position(const sys::raw_pos position);
    };
    uint64_t get_basic_font_width_text(const char *text);

    void swap_buffers(color *buffer1, const color *buffer2, uint64_t buffer_length);

    void raw_clear_buffer(color *buffer, uint64_t size, color value = {0, 0, 0, 255});
} // namespace gui
