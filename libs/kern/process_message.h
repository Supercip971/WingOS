#pragma once
#include <stdint.h>
#include <utils/wvector.h>
namespace sys
{

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
