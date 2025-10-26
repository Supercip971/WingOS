#include "arch/generic/syscalls.h"
#include "iol/wingos/space.hpp"
#include "iol/wingos/syscalls.h"
#include "libcore/fmt/log.hpp"
#include "mcx/mcx.hpp"
#include "protocols/disk/disk.hpp"
#include "protocols/init/init.hpp"
#include "protocols/vfs/fsManager.hpp"
#include "protocols/vfs/vfs.hpp"
#include "wingos-headers/syscalls.h"
#include "protocols/server_helper.hpp"
#include "ext4.hpp"

bool is_ext4_filesystem(uint8_t *data)
{
    // check for ext4 magic number at offset 0x438

    uint16_t magic = *(uint16_t *)(data + 0x38);
    return magic == 0xEF53;
}

int _main(mcx::MachineContext *)
{
    auto serv = prot::ManagedServer::create_registered_server("fs:ext4:manager", 1, 0).unwrap();

    prot::VfsConnection vfs = prot::VfsConnection::connect().unwrap();

    vfs.register_fs(core::Str("ext4"), serv.addr()).unwrap();
    log::log$("ext4: registered fs manager with vfs");
    while (true)
    {
        serv.accept_connection();
        

        auto received = serv.try_receive();
        if (received.is_error())
        {
            continue;
        }

        auto msg = received.unwrap();

        switch (msg.received.data[0].data)
        {
        case prot::DISK_FS_ATTEMPT_INITIALIZE_DISK:
        {
            IpcServerHandle endpoint = msg.received.data[1].data;
            size_t begin_lba = msg.received.data[2].data;
            size_t end_lba = msg.received.data[3].data;
            size_t part_id = msg.received.data[4].data;

            core::Str name;
            char name_buf[80];
            for (size_t i = 0; i < msg.received.len && i < 80; i++)
            {
                name_buf[i] = msg.received.raw_buffer[i];
            }
            name = core::Str(name_buf, msg.received.len);

            log::log$("ext4: mount request for device {} (part id {})", name.view(), part_id);

            auto disk_conn_res = prot::DiskConnection::connect(endpoint);
            if (disk_conn_res.is_error())
            {
                log::err$("ext4: failed to connect to disk endpoint: {}", disk_conn_res.error());
                break;
            }

            auto disk_conn = disk_conn_res.unwrap();

            // check quickly if ext4 is present


            auto dfs = Ext4Filesystem::initialize(disk_conn, begin_lba, end_lba);
            if (dfs.is_error())
            {
                log::err$("ext4: failed to initialize ext4 filesystem on device {}: {}", name.view(), dfs.error());
                
                IpcMessage reply = {};
                reply.data[0].data = 0; // fs endpoint 0 means failure
                auto send_res = serv.reply(core::move(msg), reply);
                if (send_res.is_error())
                {
                    log::err$("ext4: failed to send mount failure reply: {}", send_res.error());
                }

                break;
            }

            (void)end_lba;
            log::log$("ext4: ext4 filesystem detected on device {}, mounting...", name.view());

            while (true)
            {
            };

        }
        default:
        {
            log::warn$("ext4: unknown message type received: {}", msg.received.data[0].data);
            break;
        }
        }
    }
}