
#include <kern/file.h>
#include <kern/mouse_keyboard.h>
#include <kern/process_message.h>
#include <kern/syscall.h>
#include <stdio.h>
#include <utils/device_file_info.h>
namespace sys
{
    sys::file mouse_file = sys::file(MOUSE_FILE_BUFFER);
    sys::file keyboard_file = sys::file(KEYBOARD_FILE_BUFFER);
    inline void use_mouse_file()
    {
        if (!mouse_file.is_openned())
        {
            mouse_file.open(MOUSE_FILE_BUFFER);
        }
    }
    inline void use_keyboard_file()
    {
        if (!keyboard_file.is_openned())
        {
            keyboard_file.open(KEYBOARD_FILE_BUFFER);
        }
    }
    int32_t get_mouse_x()
    {
        use_mouse_file();
        mouse_file.seek(0);
        mouse_buff_info buff;
        mouse_file.read((uint8_t *)&buff, sizeof(mouse_buff_info));

        return buff.mouse_x;
    }

    int32_t get_mouse_y()
    {
        use_mouse_file();
        mouse_buff_info buff;
        mouse_file.seek(0);
        mouse_file.read((uint8_t *)&buff, sizeof(mouse_buff_info));

        return buff.mouse_y;
    }

    bool get_mouse_button(int button_id)
    {
        use_mouse_file();
        mouse_buff_info buff;
        mouse_file.seek(0);
        mouse_file.read((uint8_t *)&buff, sizeof(mouse_buff_info));

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
        use_keyboard_file();
        keyboard_buff_info target;
        keyboard_file.seek(id * sizeof(keyboard_buff_info));
        if (keyboard_file.read((uint8_t *)&target, sizeof(keyboard_buff_info)) == 0)
        {
            printf("unable to read offset %x", id);
        }
        return target;
    }
    size_t get_current_keyboard_offset()
    {
        use_keyboard_file();

        size_t length = keyboard_file.get_file_length() / sizeof(keyboard_buff_info);
        return length;
    }
    char get_last_key_press()
    {
        use_keyboard_file();
        keyboard_buff_info target;
        size_t length = (keyboard_file.get_file_length() / sizeof(keyboard_buff_info)) - 1;
        if (length < 0)
        {
            return 0;
        }
        keyboard_file.seek(length * sizeof(keyboard_buff_info));
        if (keyboard_file.read((uint8_t *)&target, sizeof(keyboard_buff_info)) == 0)
        {
            printf("unable to read offset %x", length);
        }
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
