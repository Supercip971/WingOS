

#include "protocols/hi/human_interface.hpp"

#include "libcore/fmt/log.hpp"
#include "libcore/result.hpp"
#include "protocols/clock/clock.hpp"
#include "protocols/humming-bird/humming-bird.hpp"
#include "protocols/vfs/vfs.hpp"

int main(int, char **)
{

    fmt::log$("Hello world from an application !");
    fmt::log$("Wingos is a microkernel based OS! made with <3");

    auto conn = prot::HBConnection::connect().unwrap();

    fmt::log$("Humming Bird: connected to posix compatibility server, pid: {}", conn.get_posix_pid());
    while (true)
    {
    }

    return 0;
}
