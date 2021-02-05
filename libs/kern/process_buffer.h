#pragma once

#include <stdint.h>
namespace sys
{
    enum process_buffer_type
    {
        NULL_PBT = -1,
        STDOUT = 0,
        STDIN = 1,
        STDERR = 2
    };
    struct raw_process_buffer_request
    {
        uint64_t m[512];
    } __attribute__((packed));

    struct get_process_buffer
    {
        int buffer_type;
        int pid_target;
        bool get_buffer_length; // if true just give the current std length else give the readed length
        uint8_t *target_buffer;
        uint64_t length_to_read;
        uint64_t where;
    } __attribute__((packed));
    struct out_process_buffer
    {
        int buffer_type;
        int pid_target;
        uint8_t *output_data;
        uint64_t length;
        uint64_t where;
    } __attribute__((packed));

    enum process_buffer_request_id
    {

        GET_PROCESS_BUFFER = 1,
        OUT_PROCESS_BUFFER = 2,
    };

    struct process_buffer_request
    {
        uint8_t type;
        union
        {
            raw_process_buffer_request raw;
            get_process_buffer gpb;
            out_process_buffer opb;
        };
    } __attribute__((packed));

    class process_buffer
    {
        uint64_t current_cursor = 0;
        process_buffer_type current_type = NULL_PBT;
        int current_pid = 0;

    public:
        process_buffer(int pid, process_buffer_type type);
        uint64_t get_length();
        uint64_t next(uint8_t *data, int length);
        void out_data(uint8_t *data, unsigned int length);
        void set_cursor(int at);
    };
} // namespace sys
