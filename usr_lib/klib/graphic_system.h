#pragma once
#include <klib/raw_graphic.h>
#include <stdint.h>
namespace sys
{
    // FOR GRAPHIC MESSAGE :
    struct graphic_raw_request
    {
        uint8_t request_raw_data[256]; // 256 o = 32 uint64_t i think we have a lot
    } __attribute__((packed));
    struct create_graphic_window
    {
        char *name;
        uint64_t width;
        uint64_t height;
        bool resizable = false; // will not be used
    } __attribute__((packed));
    struct individual_request
    {
        uint64_t window_handler_code;
    } __attribute__((packed));
    struct set_window_pos_request
    {
        uint64_t window_handler_code;
        sys::raw_pos position;
    };

    enum GRAPHIC_SYSTEM_REQUEST
    {
        NULL_REQUEST = 0,
        CREATE_WINDOW = 1,
        GET_WINDOW_BACK_BUFFER = 2,
        SWAP_WINDOW_BUFFER = 3,
        GET_WINDOW_POSITION = 4,
        SET_WINDOW_POSITION = 5
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
            graphic_raw_request raw_information; // used for later, size : 256o (2048 byte)
            create_graphic_window create_window_info;
            individual_request get_request;
            set_window_pos_request set_pos;
        };
    } __attribute__((packed));
    // FOR OTHER THING
    struct pixel
    {
        union
        {
            struct
            {
                uint8_t r;
                uint8_t g;
                uint8_t b;
                uint8_t a;
            };
            uint32_t pix;
        };
        pixel()
        {
            pix = 0;
        };
        pixel(uint32_t p)
        {
            pix = p;
        };
        pixel(uint8_t vr, uint8_t vg, uint8_t vb, uint8_t va = 255)
        {
            r = vr;
            g = vg;
            b = vb;
            a = va;
        }
    };
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
    };

    class graphic_context
    {
        uint64_t context_width;
        uint64_t context_height;
        uint64_t wid = 0;
        pixel *back_buffer;
        char *context_name;

    public:
        graphic_context(uint64_t width, uint64_t height, const char *name);
        void draw_rectangle(const uint64_t x, const uint64_t y, const uint64_t width, const uint64_t height, const pixel color);
        void draw_basic_char(const uint64_t x, const uint64_t y, const char chr, const pixel color);
        void draw_basic_string(const uint64_t x, const uint64_t y, const char *str, const pixel color);
        void clear_buffer(const pixel color);
        void swap_buffer();

        sys::raw_pos get_graphic_context_position();
        void set_graphic_context_position(const sys::raw_pos position);
    };
    uint64_t get_basic_font_width_text(const char *text);

    void swap_buffer(sys::pixel *buffer1, const sys::pixel *buffer2, uint64_t buffer_length);

    void raw_clear_buffer(sys::pixel *buffer, uint64_t size, sys::pixel value = {0, 0, 0, 255});
} // namespace sys
