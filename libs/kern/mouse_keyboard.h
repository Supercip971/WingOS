#pragma once
#include <stddef.h>
#include <stdint.h>
#include <utils/device_file_info.h>
namespace sys
{
    static const char asciiDefault[58] =
        {
            0,
            0,  // ESC
            49, // 1
            50,
            51,
            52,
            53,
            54,
            55,
            56,
            57, // 9
            48, // 0
            45, // -
            61, // =
            8,
            0,
            113, // q
            119,
            101,
            114,
            116,
            121,
            117,
            105,
            111,
            112, // p
            91,  // [
            93,  // ]
            '\n',
            10, // return
            97, // a
            115,
            100,
            102,
            103,
            104,
            106,
            107,
            108, // l
            59,  // ;
            39,  // '
            96,  // `
            0,
            92,  // BACKSLASH
            122, // z
            120,
            99,
            118,
            98,
            110,
            109, // m
            44,  // ,
            46,  // .
            47,  // /
            0,
            42, // *
            0,
            32, // SPACE
    };
    static const char asciiShift[] =
        {
            0,
            0,  // ESC
            33, // !
            64, // @
            35, // #
            36, // $
            37, // %
            94, // ^
            38, // &
            42, // *
            40, // (
            41, // )
            95, // _
            43, // +
            0,
            0,
            81, // Q
            87,
            69,
            82,
            84,
            89,
            85,
            73,
            79,
            80,  // P
            123, // {
            125, // }
            0,
            10,
            65, // A
            83,
            68,
            70,
            71,
            72,
            74,
            75,
            76,  // L
            58,  // :
            34,  // "
            126, // ~
            0,
            124, // |
            90,  // Z
            88,
            67,
            86,
            66,
            78,
            77, // M
            60, // <
            62, // >
            63, // ?
            0,
            0,
            0,
            32, // SPACE
    };
    int32_t get_mouse_x();
    int32_t get_mouse_y();
    bool get_mouse_button(int button_id);
    char get_last_key_press();

    size_t get_current_keyboard_offset();

    keyboard_buff_info get_key_press(size_t id);
    struct raw_request_data
    {

        uint8_t raw_data[32];
    } __attribute__((packed));
    struct mouse_get_position
    {
        bool get_x_value;
    } __attribute__((packed));
    struct mouse_get_button
    {
        int mouse_button_type;
    } __attribute__((packed));
    enum device_target_type
    {
        TARGET_MOUSE = 1,
        TARGET_KEYBOARD = 2,
    };

    enum keyboard_request_type
    {

        GET_KEYBOARD_KEY = 0,
    };

    struct get_keyboard_key_down
    {
        bool unused;
    } __attribute__((packed));

    struct ps2_device_request
    {
        uint8_t device_target; // for the moment 1 = mouse 2 = keyboard
        uint64_t request_type;

        union
        {
            raw_request_data data;
            mouse_get_position mouse_request_pos;
            mouse_get_button mouse_button_request;
            get_keyboard_key_down get_key_down;
        };
    } __attribute__((packed));
    enum mouse_request_type
    {
        GET_MOUSE_POSITION = 0,
        GET_MOUSE_BUTTON = 1
    };

    enum mouse_button_type
    {
        GET_MOUSE_LEFT_CLICK = 0,
        GET_MOUSE_RIGHT_CLICK = 1,
        GET_MOUSE_MIDDLE_CLICK = 2
    };
} // namespace sys
