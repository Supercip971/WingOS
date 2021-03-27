#include <kern/process_message.h>
#include <kern/syscall.h>
namespace sys
{
    service_message::service_message()
    {
        loaded = false;
    }

    service_message::service_message(const char *to, uint64_t address_to_send, uint64_t data_length)
    {
        for (int i = 0; i < 9999; i++)
        {
            source = sys$send_message(address_to_send, data_length, to);
            if (source != nullptr)
            {
                return;
            }
        }
    }

    uint64_t service_message::read()
    {
        while (true)
        {
            uint64_t response = 0;
            response = sys$message_response(source);
            if (response == -1)
            {
                return 0;
            }
            else if (response == -2)
            {
                continue;
            }
            else
            {
                return response;
            }
        }
    }
    process_message::process_message()
    {

        loaded = false;
    }
    process_message::process_message(uint64_t to_pid, uint64_t address_to_send, uint64_t data_length)
    {
        for (int i = 0; i < 9999; i++)
        {
            source = sys$send_message_pid(address_to_send, data_length, to_pid);
            if (source != nullptr)
            {
                return;
            }
        }
    }

    uint64_t process_message::read()
    {
        while (true)
        {
            uint64_t response = 0;
            response = sys$message_response(source);
            if (response == -1)
            {
                return 0;
            }
            else if (response == -2)
            {
                continue;
            }
            else
            {
                return response;
            }
        }
    }

    raw_process_message *service_read_current_queue()
    {
        return sys$read_message();
    }


server::server(const char* path) : connection_list(){
    if(sys$ipc_server_exist(path)){
        printf("can't create server %s that already exist \n");
    }
    connection_list.clear();
    server_id = sys$create_server(path);
}

int server::accept_new_connection(){
   uint32_t v = sys$accept_connection(server_id);
   if(v != 0){
       connection_list.push_back(v);
   }
   return v;
}
size_t server::send(int connection, void* data, size_t size){
    raw_msg_request request;
    request.size  = size;
    request.data = (uint8_t *)data;

    return sys$send(connection, &request, 0); // no flag for the moment
}
size_t server::receive(int connection, void* data, size_t size){
    raw_msg_request request;
    request.size  = size;
    request.data = (uint8_t *)data;

    return sys$receive(connection, &request, 0); // no flag for the moment

}

client_connection::client_connection(){
    id = 0;
    accepted = false;
}
client_connection::client_connection(const char* target){
    id = sys$connect_to_server(target);
    accepted = false;
}
bool client_connection::deconnect(){
    if(id){
        return sys$deconnect(id);
    }else{
        return false;
    }
}
bool client_connection::is_accepted(){
    if(id){

        return sys$is_connection_accepted(id);
    }else{
        return false;
    }

}


size_t client_connection::send( const void* data, size_t size){
    raw_msg_request request;
    request.size  = size;
    request.data = (uint8_t *)data;

    return sys$send(id, &request, 0); // no flag for the moment
}
size_t client_connection::receive( void* data, size_t size){
    raw_msg_request request;
    request.size  = size;
    request.data = (uint8_t *)data;

    return sys$receive(id, &request, 0); // no flag for the moment

}
} // namespace sys
