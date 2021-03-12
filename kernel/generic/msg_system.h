#ifndef MSG_SYSTEM_H
#define MSG_SYSTEM_H
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <utils/alloc_array.h>
#include <utils/config.h>
#include <utils/smart_ptr.h>
#include <utils/warray.h>
#include <utils/wvector.h>
struct raw_msg_request
{
    bool valid;
    size_t size;
    uint8_t *data;
};
union raw_msg_connection
{
    uint32_t raw;
    struct
    {
        uint16_t connection_id;
        uint16_t server_id;
    };
};

raw_msg_request copy_request(const raw_msg_request from);
class msg_queue
{

    utils::vector<raw_msg_request> raw_queue;

public:
    utils::vector<raw_msg_request> &get()
    {
        return raw_queue;
    }
    const utils::vector<raw_msg_request> &get() const
    {
        return raw_queue;
    }

    bool has_in_msg() const
    {
        return raw_queue.size() != 0;
    }
    raw_msg_request consume_msg()
    {
        raw_msg_request req = raw_queue.get(0);
        raw_queue.remove(0);
        return req;
    }
    raw_msg_request &get_last_msg()
    {
        return raw_queue.get(0);
    }
    bool send_msg(const raw_msg_request request)
    {
        raw_queue.push_back(copy_request(request));
        return true;
    }
    void destroy(){};
};

class msg_connection
{
    int _from_pid;
    int _to_msg_system;
    bool _accepted;
    int _id;
    msg_queue _out;
    msg_queue _in;

public:
    msg_connection()
    {
        _accepted = false;
    }
    msg_connection(int connection_id, int pid, int msg_sys_id) : _from_pid(pid), _to_msg_system(msg_sys_id), _id(connection_id)
    {
        _accepted = false;
    }
    void accepted(bool status) { _accepted = status; };
    bool accepted() const { return _accepted; };

    bool destroy()
    {
        _out.destroy();
        _in.destroy();
        return true;
    }

    int id() const { return _id; };
    int from_pid() const { return _from_pid; };
    msg_queue &out_queue() { return _out; };
    msg_queue &in_queue() { return _in; };
};

class msg_system
{
    int next_connection_uid = 10;
    static constexpr int max_connection = SERVER_MAX_CONNECTION;
    utils::alloc_array<msg_connection, max_connection> connection_list;
    utils::unique_ptr<char> name;
    int connection_waiting_count = 0;
    int msg_system_id;
    size_t by_server_pid;
    int get_connection_table_id(int connection_id);

public:
    msg_system()
    {
        msg_system_id = -1;
        by_server_pid = -1;
        name = utils::unique_ptr<char>(nullptr);
    }

    void set(const char *path, int id, size_t pid_server)
    {
        next_connection_uid = 10;
        name = new char[strlen(path) + 1];
        msg_system_id = id;
        by_server_pid = (pid_server);
        memcpy(name.get_raw(), path, strlen(path) + 1);
    }
    void rename(const char *new_name)
    {
        name.reset(new char[strlen(new_name) + 1]);
        memcpy(name.get_raw(), new_name, strlen(new_name) + 1);
    };

    const char *msg_name() const { return name.get_raw(); };

    bool is_name(const char *comp) const
    {
        if (strcmp(name.get_raw(), comp) == 0)
        {
            return true;
        }
        return false;
    };

    int get_msg_system_id() const { return msg_system_id; };

    utils::alloc_array<msg_connection, max_connection> &connections() { return connection_list; };

    int accept_connection();

    int add_connection(int pid);

    msg_connection *get_connection(int msg_id);

    bool deconnect(int connection_id, int pid);

    bool valid_connection(int id, int pid);

    size_t get_server_pid() const { return by_server_pid; };

    int send(int connection_id, const raw_msg_request request, int flags, size_t by_pid);
    int receive(int connection_id, raw_msg_request request, int flags, size_t by_pid);
};

void init_msg_system();

bool service_exist(const char *path);
int create_msg_system(const char *path, size_t by_pid);

size_t accept_connection(int service_id);
uint32_t connect(const char *msg_system, size_t by_pid);
bool connection_accepted(uint32_t id);
int deconnect(uint32_t id);

size_t send(uint32_t id, const raw_msg_request *request, int flags);
size_t receive(uint32_t id, raw_msg_request *request, int flags);

#endif // MSG_SYSTEM_H
