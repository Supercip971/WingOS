
#include <kern/file.h>
#include <kern/mouse_keyboard.h>
#include <kern/process_message.h>
#include <kern/syscall.h>
#include <stdio.h>
#include <utils/device_file_info.h>
namespace sys
{

    int32_t get_mouse_x()
    {
        sys::file mouse_file = sys::file(MOUSE_FILE_BUFFER);
        mouse_file.seek(0);
        mouse_buff_info buff;
        mouse_file.read((uint8_t *)&buff, sizeof(mouse_buff_info));

        mouse_file.close();
        return buff.mouse_x;
    }

    int32_t get_mouse_y()
    {
        sys::file mouse_file = sys::file(MOUSE_FILE_BUFFER);
        mouse_buff_info buff;
        mouse_file.read((uint8_t *)&buff, sizeof(mouse_buff_info));

        mouse_file.close();
        return buff.mouse_y;
    }

    bool get_mouse_button(int button_id)
    {
        sys::file mouse_file = sys::file(MOUSE_FILE_BUFFER);
        mouse_buff_info buff;
        mouse_file.read((uint8_t *)&buff, sizeof(mouse_buff_info));

        mouse_file.close();
        if (button_id == mouse_button_type::GET_MOUSE_LEFT_CLICK)
        {
            return buff.left;
        }
        if (button_id == mouse_button_type::GET_MOUSE_RIGHT_CLICK)
        {
            return buff.right;
        }
        if (button_id == mouse_button_type::GET_MOUSE_MIDDLE_CLICK)
        {
            return buff.middle;
        }
        return false;
    }

    keyboard_buff_info get_key_press(size_t id)
    {

        sys::file keybfile = sys::file(KEYBOARD_FILE_BUFFER);
        keyboard_buff_info target;
        keybfile.seek(id * sizeof(keyboard_buff_info));
        if (keybfile.read((uint8_t *)&target, sizeof(keyboard_buff_info)) == 0)
        {
            printf("unable to read offset %x", id);
        }
        keybfile.close();
        return target;
    }
    size_t get_current_keyboard_offset()
    {

        sys::file keybfile = sys::file(KEYBOARD_FILE_BUFFER);
        size_t length = keybfile.get_file_length() / sizeof(keyboard_buff_info);
        keybfile.close();
        return length;
    }
    char get_last_key_press()
    {
        sys::file keybfile = sys::file(KEYBOARD_FILE_BUFFER);
        keyboard_buff_info target;
        size_t length = (keybfile.get_file_length() / sizeof(keyboard_buff_info)) - 1;
        if (length < 0)
        {
            return 0;
        }
        keybfile.seek(length * sizeof(keyboard_buff_info));
        if (keybfile.read((uint8_t *)&target, sizeof(keyboard_buff_info)) == 0)
        {
            printf("unable to read offset %x", length);
        }
        keybfile.close();
        if (target.state == 1)
        {

            return target.button;
        }
        else
        {
            return 0;
        }
    }
} // namespace sys
