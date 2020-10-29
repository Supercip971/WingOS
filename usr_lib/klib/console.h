#pragma once
#include <stdint.h>
namespace sys
{
    enum console_request_type
    {
        CONSOLE_WRITE = 1,
        CONSOLE_READ
    };

    struct read_console
    {
        uint64_t pid_target;
        uint64_t at;
        uint64_t length;
    };
    struct write_console_data
    {
        char raw_data[512]; // null terminated
    };

    struct console_service_request
    {
        uint8_t request_type;
        union
        {
            read_console read;
            write_console_data write;
        };
    } __attribute__((packed));

} // namespace sys
