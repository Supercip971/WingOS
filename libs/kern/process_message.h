#pragma once
#include <stdint.h>
#include <utils/wvector.h>
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
    class service_message
    {
        raw_process_message *source;
        bool loaded = false;

    public:
        service_message();
        service_message(const char *to, uint64_t address_to_send, uint64_t data_length); // will be send automatically

        uint64_t read(); // will be read
    };
    class process_message
    {
        raw_process_message *source;
        bool loaded = false;

    public:
        process_message();
        process_message(uint64_t to_pid, uint64_t address_to_send, uint64_t data_length); // will be send automatically

        uint64_t read(); // will be read
    };

    raw_process_message *service_read_current_queue();

    class server{
        int server_id;
        utils::vector<uint32_t> connection_list;
    public:
        server() = default;
        server(const char* path);

        int accept_new_connection();

        size_t send(int connection, void* data, size_t size);
        size_t receive(int connection, void* data, size_t size);

        utils::vector<uint32_t>& get_connection_list(){return connection_list;};
    };


    class client_connection{
    protected:
        int id;
        bool accepted;
    public:
        client_connection();
        client_connection(const char* target);
        bool is_accepted();
        void wait_accepted(){ while (!is_accepted()) {} };

        bool deconnect();

        size_t send(const void* data, size_t size);
        size_t receive( void* data, size_t size);

        size_t wait_receive( void* data, size_t size){
            size_t res = receive(data, size);
            while (res != size) {
                res = receive(data, size);
            }
            return res;
        }

    };
} // namespace sys
