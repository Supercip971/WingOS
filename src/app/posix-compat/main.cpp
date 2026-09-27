#include <string.h>

#include "protocols/server_helper.hpp"

#include "protocols/init/init.hpp"
#include "server.hpp"

pid_t posix_pid_counter = 1;

int main(int, char **)
{
    auto serv_g = prot::ManagedServer::create_registered_server<PosixServer>("humming-bird", 1, 0);
    auto serv = serv_g.take();

    fmt::log$("Humming Bird: posix compatibility server started");

    while (true)
    {
        // alive.tick();
        serv->loop();
    }
    return 0;
}
