#include "protocols/server_helper.hpp"

#include "app/fs/ext4/server.hpp"
#include "ext4.hpp"
#include "iol/wingos/asset.hpp"
#include "libcore/ds/vec.hpp"
#include "libcore/fmt/log.hpp"
#include "libcore/result.hpp"
#include "libcore/type-utils.hpp"
#include "protocols/disk/disk.hpp"
#include "protocols/vfs/file.hpp"
#include "protocols/vfs/fsManager.hpp"
#include "protocols/vfs/vfs.hpp"
#include "wingos-headers/ipc.h"

bool is_ext4_filesystem(uint8_t *data)
{
    // check for ext4 magic number at offset 0x438

    uint16_t magic = *(uint16_t *)(data + 0x38);
    return magic == 0xEF53;
}

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
