#include "msg_system.h"
#include <logging.h>
#include <process.h>
#include <utils/lock.h>
#include <utils/math.h>
int msg_system_id = 10;
utils::alloc_array<msg_system, MAX_SERVER_COUNT> msg_system_list;
utils::lock_type system_lock;
msg_system *get_msg_system(uint32_t id)
{
    utils::context_lock locker(system_lock);
    for (size_t i = 0; i < msg_system_list.size(); i++)
    {
        if (msg_system_list.status(i) && (msg_system_list[i].get_msg_system_id() == (uint32_t)id))
        {
            return &msg_system_list[i];
        }
    }
    return nullptr;
}
msg_system *get_msg_system(const char *val)
{
    utils::context_lock locker(system_lock);
    for (size_t i = 0; i < msg_system_list.size(); i++)
    {
        if (msg_system_list.status(i) && (strcmp(val, msg_system_list[i].msg_name()) == 0))
        {
            return &msg_system_list[i];
        }
    }
    return nullptr;
}
bool is_valid_msg_system(size_t pid, int msg_id)
{
    auto res = get_msg_system(msg_id);
    if (res == nullptr)
    {
        return false;
    }
    else if (res->get_server_pid() != pid)
    {
        return false;
    }
    return true;
}
raw_msg_request copy_request(const raw_msg_request from)
{
    raw_msg_request target;
    target.valid = from.valid;
    target.size = from.size;
    target.data = (uint8_t *)malloc(from.size);
    memcpy(target.data, from.data, from.size);
    return target;
}

int create_msg_system(const char *path, size_t by_pid)
{
    size_t res = msg_system_list.alloc();
    msg_system_list[res].set(path, msg_system_id, by_pid);
    log("msg system", LOG_INFO, "created server {}/{} by process {}", path, msg_system_id, by_pid);

    msg_system_id++;
    return msg_system_id - 1;
}

size_t accept_connection(int service_id)
{
    auto v = get_msg_system(service_id);
    if (v == nullptr)
    {
        log("msg system", LOG_ERROR, "trying to connect to accept system {} that does not exist", service_id);
        return 0;
    }
    if (v->get_server_pid() != process::current()->get_parent_pid())
    {
        log("msg system", LOG_ERROR, "process trying ot accept connection from a reserved service by another process");
        return 0;
    }
    uint32_t res = v->accept_connection();

    if (res == 0)
    {
        return 0;
    }
    raw_msg_connection connect = {};
    connect.element.connection_id = res;
    connect.element.server_id = service_id;
    return connect.raw;
}
bool service_exist(const char *path)
{
    auto v = get_msg_system(path);
    if (v == nullptr)
    {
        return 0;
    }
    return 1;
}

bool connection_accepted(uint32_t id)
{

    raw_msg_connection msg_connection;
    msg_connection.raw = id;
    auto v = get_msg_system(msg_connection.element.server_id);
    if (v == nullptr)
    {
        log("msg system", LOG_ERROR, "trying to get if a connection is accepted from message system {} that does not exist", msg_connection.element.server_id);
        return 0;
    }
    return v->get_connection(msg_connection.element.connection_id)->accepted();
}
uint32_t connect(const char *msg_system, size_t by_pid)
{
    auto v = get_msg_system(msg_system);
    if (v == nullptr)
    {
        log("msg system", LOG_ERROR, "trying to connect to message system {} that does not exist", msg_system);
        return 0;
    }
    raw_msg_connection msg_connection;
    msg_connection.element.server_id = v->get_msg_system_id();
    msg_connection.element.connection_id = v->add_connection(by_pid);

    if (msg_connection.element.connection_id == 0)
    {
        log("msg system", LOG_ERROR, "server refuse connection");
        return 0;
    }
    return msg_connection.raw;
}

int deconnect(uint32_t id)
{
    raw_msg_connection msg_connection;
    msg_connection.raw = id;
    auto v = get_msg_system(msg_connection.element.server_id);
    if (v == nullptr)
    {
        log("msg system", LOG_ERROR, "trying to deconnect to message system {} that does not exist", msg_connection.element.server_id);
        return 1;
    }

    if (v->deconnect(msg_connection.element.connection_id, process::current()->get_parent_pid()))
    {
        return 0;
    }
    else
    {
        log("msg system", LOG_ERROR, "can't deconnect to message system {} with connection {}", msg_connection.element.server_id, msg_connection.element.connection_id);
        return 2;
    }
}
size_t send(uint32_t id, const raw_msg_request *request, int flags)
{

    raw_msg_connection msg_connection;
    msg_connection.raw = id;
    auto v = get_msg_system(msg_connection.element.server_id);
    if (v == nullptr)
    {
        log("msg system", LOG_ERROR, "trying to send to message system {}/{} that does not exist", msg_connection.element.server_id, msg_connection.element.connection_id);
        return 0;
    }
    return (size_t)v->send(msg_connection.element.connection_id, *request, flags, process::current()->get_parent_pid());
}
size_t receive(uint32_t id, raw_msg_request *request, int flags)
{

    raw_msg_connection msg_connection;
    msg_connection.raw = id;
    auto v = get_msg_system(msg_connection.element.server_id);
    if (v == nullptr)
    {
        log("msg system", LOG_ERROR, "trying to receive from message system {}/{} that does not exist", msg_connection.element.server_id, msg_connection.element.connection_id);
        return 0;
    }
    return (size_t)v->receive(msg_connection.element.connection_id, *request, flags, process::current()->get_parent_pid());
}
void init_msg_system()
{
}

uint32_t msg_system::accept_connection()
{
    if (connection_waiting_count == 0)
    {
        return 0;
    }
    for (size_t i = 0; i < connection_list.size(); i++)
    {
        msg_system_lock.lock();
        if (connection_list.status(i) && !connection_list[i].accepted())
        {
            connection_list[i].accepted(true);
            connection_waiting_count--;
            msg_system_lock.unlock();
            return connection_list[i].id();
        }
        msg_system_lock.unlock();
    }

    return 0;
}

int msg_system::add_connection(int pid)
{
    utils::context_lock locker(msg_system_lock);
    msg_connection connect(next_connection_uid++, pid, msg_system_id);
    size_t v = connection_list.alloc();
    if (connection_list.status(v))
    {
        connection_list[v] = connect;
        connection_waiting_count++;
        return connect.id();
    }
    else
    {
        log("msg system", LOG_ERROR, "can not alloc message for msg system");
        return 0;
    }
}

bool msg_system::valid_connection(int id, int pid)
{
    auto conn = get_connection(id);
    if (conn == nullptr)
    {
        return false;
    }
    return conn->from_pid() == pid;
}

msg_connection *msg_system::get_connection(uint32_t connection_id)
{
    utils::context_lock locker(msg_system_lock);
    for (size_t i = 0; i < connection_list.size(); i++)
    {
        if (connection_list.status(i) && connection_list[i].id() == connection_id)
        {
            return &connection_list.get(i);
        }
    }
    return nullptr;
}

int msg_system::get_connection_table_id(uint32_t connection_id)
{

    utils::context_lock locker(msg_system_lock);
    for (size_t i = 0; i < connection_list.size(); i++)
    {
        if (connection_list.status(i) && connection_list[i].id() == connection_id)
        {
            return i;
        }
    }
    return -1;
}
bool msg_system::deconnect(int connection_id, int pid)
{
    if (!valid_connection(connection_id, pid))
    {
        log("msg system", LOG_ERROR, "trying to deconnect a not valid connection: {} by pid: {}", connection_id, pid);
        return false;
    }
    auto val = get_connection_table_id(connection_id);
    utils::context_lock locker(msg_system_lock);
    connection_list.free(val);
    return true;
}
int msg_system::send(int connection_id, const raw_msg_request request, int flags, size_t by_pid)
{
    if (by_pid == by_server_pid)
    {
        auto connection = get_connection(connection_id);
        utils::context_lock locker(msg_system_lock);
        connection->in_queue().send_msg(request);
        return request.size;
    }
    else if (valid_connection(connection_id, by_pid))
    {

        auto connection = get_connection(connection_id);
        utils::context_lock locker(msg_system_lock);
        if (!connection->accepted())
        {
            log("msg system", LOG_ERROR, "invalid send to connection: {} not accepted by pid: {}", connection_id, by_pid);
            return 0;
        }
        connection->out_queue().send_msg(request);
        return request.size;
    }
    else
    {
        log("msg system", LOG_ERROR, "invalid send to connection: {} from pid: {}", connection_id, by_pid);
        return 0;
    }
}

int msg_system::receive(int connection_id, raw_msg_request request, int flags, size_t by_pid)
{

    if (by_pid == by_server_pid)
    {
        auto connection = get_connection(connection_id);
        if (connection == nullptr)
        {
            log("msg system", LOG_ERROR, "invalid connection receive {}", connection_id);
            return 0;
        }
        if (connection->out_queue().get().size() == 0)
        {
            return 0;
        }
        msg_system_lock.lock();
        auto last_msg = connection->out_queue().get_last_msg();
        size_t readed_length = utils::min(last_msg.size, request.size);

        if (readed_length != last_msg.size)
        {
            if (request.data != nullptr)
            {
                memcpy(request.data, last_msg.data, readed_length);
            }
        }
        else
        {
            if (request.data != nullptr)
            {
                memcpy(request.data,
                       connection->out_queue().consume_msg().data, readed_length);
            }
            else
            {
                connection->out_queue().consume_msg();
            }
            free(last_msg.data);
        }
        msg_system_lock.unlock();

        return readed_length;
    }
    else if (valid_connection(connection_id, by_pid))
    {
        auto connection = get_connection(connection_id);

        if (connection == nullptr)
        {
            log("msg system", LOG_ERROR, "invalid connection receive {}", connection_id);
            return 0;
        }
        if (connection->in_queue().get().size() == 0)
        {
            return 0;
        }
        msg_system_lock.lock();
        auto last_msg = connection->in_queue().get_last_msg();
        size_t readed_length = utils::min(last_msg.size, request.size);

        if (!connection->accepted())
        {
            log("msg system", LOG_ERROR, "invalid send to connection: {} not accepted by pid: {}", connection_id, by_pid);
            msg_system_lock.unlock();
            return 0;
        }
        if (readed_length != last_msg.size)
        {
            if (request.data != nullptr)
            {

                memcpy(request.data, last_msg.data, readed_length);
            }
        }
        else
        {
            if (request.data != nullptr)
            {

                memcpy(request.data,
                       connection->in_queue().consume_msg().data, readed_length);
            }
            else
            {
                connection->in_queue().consume_msg();
            }
            free(last_msg.data);
        }
        msg_system_lock.unlock();

        return readed_length;
    }
    else
    {
        log("msg system", LOG_ERROR, "invalid receive to connection: {} from pid: {}", connection_id, by_pid);
        return 0;
    }
}
