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
core::Vec<Wingos::IpcConnection *> connections;

bool is_ext4_filesystem(uint8_t *data)
{
    // check for ext4 magic number at offset 0x438

    uint16_t magic = *(uint16_t *)(data + 0x38);
    return magic == 0xEF53;
}
int _main(mcx::MachineContext *)
{

    // attempt connection to server ID 0

    auto serv = Wingos::Space::self().create_ipc_server();

    auto conn = prot::InitConnection::connect().unwrap();

    // now do a call

    prot::InitRegisterServer reg = {};
    core::Str("fs:ext4:manager").copy_to((char *)reg.name, 80);

    reg.major = 1;
    reg.minor = 0;
    reg.endpoint = serv.addr;

    conn.register_server(reg).unwrap();

    prot::VfsConnection vfs = prot::VfsConnection::connect().unwrap();

    vfs.register_fs(core::Str("ext4"), serv.addr).unwrap();
    log::log$("ext4: registered fs manager with vfs");
    while (true)
    {
        auto c = serv.accept();
        if (!c.is_error())
        {
            log::log$("(server) accepted connection: {}", c.unwrap()->handle);
            connections.push(c.unwrap());
        }

        auto received = serv.receive();
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

            Wingos::MemoryAsset asset = Wingos::Space::self().allocate_physical_memory(4096);

            auto read_res = disk_conn.read(asset, (begin_lba) + 0x400 / 512, 512);
            if (read_res.is_error())
            {
                log::err$("ext4: failed to read from disk: {}", read_res.error());
                break;
            }

            Wingos::VirtualMemoryAsset vasset = Wingos::Space::self().map_memory(asset, ASSET_MAPPING_FLAG_READ | ASSET_MAPPING_FLAG_WRITE);
            if (!is_ext4_filesystem((uint8_t *)asset.memory.start() + USERSPACE_VIRT_BASE))
            {
                log::log$("ext4: not an ext4 filesystem on device {}", name.view());

                IpcMessage reply = {};
                reply.data[0].data = 0; // mount failed
                serv.reply(core::move(msg), reply).assert();

                break;
            }

            (void)end_lba;
            (void)vasset;
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