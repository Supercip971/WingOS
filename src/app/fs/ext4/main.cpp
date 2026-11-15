#include "protocols/server_helper.hpp"

#include "app/fs/ext4/server.hpp"
#include "libcore/fmt/log.hpp"
#include "libcore/result.hpp"
#include "protocols/vfs/vfs.hpp"

int main(int, char **)
{
    // auto serv_r = prot::ManagedServer::crea("fs:ext4:manager", 1, 0);
    //
    auto serv_r = prot::ManagedServer::create_registered_server<Ext4CoreServer>("fs:ext4:manager");

    auto serv = serv_r.take();

    prot::VfsConnection vfs = prot::VfsConnection::connect().take();

    vfs.register_fs(fc::Str("ext4"), serv->addr()).unwrap();
    fmt::log$("ext4: registered fs manager with vfs");
    fmt::log$("ext4: entering main loop");
    while (true)
    {
        serv->loop();
    }
}
