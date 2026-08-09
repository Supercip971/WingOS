
#include "app/fs/vfs/server.hpp"
#include "libcore/fmt/log.hpp"
#include "libcore/str.hpp"

int main(int, char **)
{

    // attempt connection to server ID 0
    fmt::log$("started vfs");

    auto server_r = VfsServer::create_registered_server<VfsServer>("vfs");
    fmt::log$("registered vfs");
    if (server_r.is_error())
    {
        fmt::err$("failed to create registered vfs server: {}", server_r.error());
        return 1;
    }

    auto server = server_r.take();
    server->loop();
}
