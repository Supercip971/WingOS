#pragma once

#include <stdint.h>
namespace sys
{
    enum process_buffer_type
    {
        STDOUT = 0,
        STDIN = 1, // not supported
        STDERR = 2
    };
    class process_buffer
    {
        uint64_t current_cursor;
        process_buffer_type current_type;
        int current_pid;

    public:
        process_buffer(int pid, process_buffer_type type);
        uint64_t get_length();
        uint64_t next(uint8_t *data, int length);
        void set_cursor(int at);
    };
} // namespace sys
