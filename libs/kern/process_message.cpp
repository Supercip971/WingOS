#include <kern/process_message.h>
#include <kern/syscall.h>
namespace sys
{

    server::server(const char *path) : connection_list()
    {
        if (sys$ipc_server_exist(path))
        {
            printf("can't create server %s that already exist \n");
        }
        connection_list.clear();
        server_id = sys$create_server(path);
    }

    int server::accept_new_connection()
    {
        uint32_t v = sys$accept_connection(server_id);
        if (v != 0)
        {
            connection_list.push_back(v);
        }
        return v;
    }
    size_t server::send(int connection, void *data, size_t size)
    {
        raw_msg_request request;
        request.size = size;
        request.data = (uint8_t *)data;

        return sys$send(connection, &request, 0); // no flag for the moment
    }
    size_t server::receive(int connection, void *data, size_t size)
    {
        raw_msg_request request;
        request.size = size;
        request.data = (uint8_t *)data;

        return sys$receive(connection, &request, 0); // no flag for the moment
    }

    client_connection::client_connection()
    {
        id = 0;
        accepted = false;
    }
    client_connection::client_connection(const char *target)
    {
        id = sys$connect_to_server(target);
        accepted = false;
    }
    bool client_connection::deconnect()
    {
        if (id)
        {
            return sys$deconnect(id);
        }
        else
        {
            return false;
        }
    }
    bool client_connection::is_accepted()
    {
        if (id)
        {

            return sys$is_connection_accepted(id);
        }
        else
        {
            return false;
        }
    }

    size_t client_connection::send(const void *data, size_t size)
    {
        raw_msg_request request;
        request.size = size;
        request.data = (uint8_t *)data;

        return sys$send(id, &request, 0); // no flag for the moment
    }
    size_t client_connection::receive(void *data, size_t size)
    {
        raw_msg_request request;
        request.size = size;
        request.data = (uint8_t *)data;

        return sys$receive(id, &request, 0); // no flag for the moment
    }
} // namespace sys
