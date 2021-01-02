#ifndef SHELL_H
#define SHELL_H
#include <stdint.h>
namespace sys
{
    enum shell_action
    {
        SHELL_SEND_KEY = 0,
        SHELL_SEND_COMMAND = 1,
        SHELL_EXIT = 2, // or any invalid command
    };
    struct shell_raw
    {
        char raw_data[64];
    };

    struct shell_send_key
    {
        char keycode;
        uint64_t advanced_key; // may be used later for 'control' 'shift' ...
    } __attribute__((packed));
    struct shell_send_command
    {
        char *command; // null terminated string
    } __attribute__((packed));
    struct shell_exit
    {
        bool force; // not supported for the moment
    } __attribute__((packed));
    struct shell_protocol
    {
        uint8_t action;
        union
        {
            shell_send_key key;
            shell_send_command command;
            shell_exit exit;
            shell_raw raw;
        };
    } __attribute__((packed));

    class shell
    {
        uint64_t shell_pid;

    public:
        shell();
        constexpr uint64_t get_shell_upid() const
        {
            return shell_pid;
        }
        void send_key_tap(char key);
        void exit();
        void send_command(const char *command);
    };
} // namespace sys

#endif // SHELL_H
