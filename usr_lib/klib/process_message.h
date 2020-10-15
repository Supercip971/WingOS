#pragma once
#include <stdint.h>

namespace sys
{
    struct raw_process_message
    {
        uint8_t message_id;
        uint64_t from_pid;
        uint64_t to_pid;
        uint64_t content_address; // NOTE IT COPY THE CONTENT
        uint64_t content_length;
        uint64_t response;
        bool has_been_readed;
        bool entry_free_to_use;
    } __attribute__((packed));
    class process_message
    {
        raw_process_message *source;
        bool loaded = false;

    public:
        process_message();
        process_message(const char *to, uint64_t address_to_send, uint64_t data_length); // will be send automatically

        uint64_t read(); // will be read
    };

    raw_process_message *service_read_current_queue();

} // namespace sys
