#pragma once

#include <sys/types.h>

#include "iol/wingos/ipc.hpp"
#include "iol/wingos/space.hpp"
#include "libcore/result.hpp"
#include "protocols/init/init.hpp"

/**
 * Humming Bird Protocol - meant as a posix compatibility layer
 */
namespace prot
{
enum
{
    HB_POSIX_REGISTER_SELF = 0,
    HB_POSIX_GETPID = 1,
    HB_POSIX_FORK = 2,
};

class HBConnection
{

    Wingos::IpcClient connection;
    pid_t _pid;

    void _register_process()
    {
        IpcMessage msg = {};
        msg.arg(0, HB_POSIX_REGISTER_SELF);
        connection.call(msg);

        msg.arg(0, HB_POSIX_GETPID);
        connection.call(msg);
        _pid = msg.arg(0);
    }

public:
    Wingos::IpcClient &raw_client() { return connection; }

    auto get_posix_pid() const { return _pid; }

    static fc::Result<HBConnection> connect()
    {
        HBConnection conn{};

        auto reg = try$(InitConnection::connect());
        auto handle = try$(reg.get_server(fc::Str("humming-bird"), 1, 0)).endpoint;
        conn.connection = Wingos::Space::self().connect_by_addr(handle);

        reg.end();

        conn._register_process();
        return conn;
    }
};
} // namespace prot