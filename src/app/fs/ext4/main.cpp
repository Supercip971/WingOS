#include "protocols/server_helper.hpp"

#include "arch/generic/syscalls.h"
#include "ext4.hpp"
#include "iol/wingos/space.hpp"
#include "iol/wingos/syscalls.h"
#include "libcore/fmt/log.hpp"
#include "mcx/mcx.hpp"
#include "protocols/disk/disk.hpp"
#include "protocols/init/init.hpp"
#include "protocols/vfs/fsManager.hpp"
#include "protocols/vfs/vfs.hpp"
#include "wingos-headers/syscalls.h"

bool is_ext4_filesystem(uint8_t *data)
{
    // check for ext4 magic number at offset 0x438

    uint16_t magic = *(uint16_t *)(data + 0x38);
    return magic == 0xEF53;
}

struct Ext4FileEndpoint
{
    Ext4InodeRef inode;
    prot::ManagedServer root_server;
    Ext4Filesystem *attached_fs;
};

core::Vec<Ext4FileEndpoint *> ext4_file_roots;
core::Vec<Ext4FileEndpoint *> ext4_file_endpoints;

bool update_endpoint(Ext4FileEndpoint *endpoint)
{

    endpoint->root_server.accept_connection();

    auto received = endpoint->root_server.try_receive();

    if (received.is_error())
    {
        return false;
    }

    auto msg = received.unwrap();

    switch (msg.received.data[0].data)
    {
    case prot::FS_OPEN_FILE:
    {

        IpcMessage reply = {};
        core::Str path;
        char path_buf[256];
        for (size_t i = 0; i < msg.received.len && i < 256; i++)
        {
            path_buf[i] = msg.received.raw_buffer[i];
        }
        path = core::Str(path_buf, msg.received.len);
        log::log$("ext4: open file request for path: {}", path.view());

        auto file_res = endpoint->attached_fs->get_subdir(
            endpoint->inode, path);

        if (file_res.is_error())
        {
            log::err$("ext4: failed to open file {}: {}", path.view(), file_res.error());
            reply.data[0].data = 0; // failure
            reply.data[1].data = 0;
        }
        else
        {
            auto file_inode = file_res.unwrap();
            log::log$("ext4: opened file {} with inode {}", path.view(), file_inode.inode_id);
            // create new endpoint for this file
            ext4_file_endpoints.push(new Ext4FileEndpoint{

                .inode = file_inode,
                .root_server = prot::ManagedServer::create_server().unwrap(),
                .attached_fs = endpoint->attached_fs,
            });

            reply.data[0].data = 1; // success

            reply.data[1].data = ext4_file_endpoints[ext4_file_endpoints.len() - 1]->root_server.addr();

            return true;
        }

        break;
    }
    case prot::FS_READ:
    {
        size_t offset = msg.received.data[1].data;
        size_t len = msg.received.data[2].data;

        log::log$("ext4: read request for inode {} offset {} len {}", endpoint->inode.inode_id, offset, len);

        auto mem_asset = Wingos::MemoryAsset::from_handle(msg.received.data[3].asset_handle);
        endpoint->attached_fs->inode_read(
            endpoint->inode,
            mem_asset,
            len,
            offset / endpoint->attached_fs->block_size());

        IpcMessage reply = {};
        reply.data[0].data = 1; // success
        reply.data[1].data = len;
        reply.data[2].asset_handle = mem_asset.handle;
        reply.data[2].is_asset = true;
        endpoint->root_server.reply(core::move(msg), reply);

        break;
    }
    default:
    {
        log::warn$("ext4: unknown message type received: {}", msg.received.data[0].data);
        break;
    }
    }
    return false;
};

void update_endpoints()
{
    for (auto &endpoint : ext4_file_endpoints)
    {
        if (update_endpoint(endpoint))
        {
            return;
        }
    }
    for (auto &root : ext4_file_roots)
    {
        if (update_endpoint(root))
        {
            return;
        }
    }
}

int _main(mcx::MachineContext *)
{
    auto serv = prot::ManagedServer::create_registered_server("fs:ext4:manager", 1, 0).unwrap();

    prot::VfsConnection vfs = prot::VfsConnection::connect().unwrap();

    vfs.register_fs(core::Str("ext4"), serv.addr()).unwrap();
    log::log$("ext4: registered fs manager with vfs");
    while (true)
    {

        update_endpoints();
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

            auto dfs_res = dfs.unwrap();

            ext4_file_endpoints.push(new Ext4FileEndpoint{

                .inode = dfs_res.read_inode(2).unwrap(),
                .root_server = prot::ManagedServer::create_server().unwrap(),
                .attached_fs = new Ext4Filesystem(dfs.unwrap())});

            IpcMessage reply = {};
            reply.data[0].data = 1; // success

            reply.data[1].data = ext4_file_endpoints[ext4_file_endpoints.len() - 1]->root_server.addr();
            (void)end_lba;
            log::log$("ext4: ext4 filesystem detected on device {}, mounting...", name.view());

            auto send_res = serv.reply(core::move(msg), reply);
            if (send_res.is_error())
            {
                log::err$("ext4: failed to send mount success reply: {}", send_res.error());
            }
            break;
        }
        default:
        {
            log::warn$("ext4: unknown message type received: {}", msg.received.data[0].data);
            break;
        }
        }

    }
}