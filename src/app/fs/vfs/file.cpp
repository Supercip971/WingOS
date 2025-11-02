#include "file.hpp"

core::Vec<MountedFs> mounted_filesystems = {};
core::Vec<VfsFileEndpoint *> opened_file_endpoints = {};

MountedFs _root;

core::Result<void> mount_fs(IpcServerHandle device_name, core::WStr &&mount_path)
{
    for (auto &mnt : mounted_filesystems)
    {
        if (mnt.path == mount_path)
        {
            return ("mount path already in use");
        }
    }

    if (mount_path.view() == core::Str("/"))
    {
        log::log$("VFS: signaled init that root fs is available");
        prot::InitConnection init_conn = prot::InitConnection::connect().unwrap();
        init_conn.signal_fs_available().unwrap();
    }

    MountedFs mnt = {};
    mnt.endpoint = device_name;
    mnt.path = core::move(mount_path);
    mounted_filesystems.push(core::move(mnt));

    return {};
}

core::Result<VfsFileEndpoint *> VfsFileEndpoint::open_root()
{
    for (size_t i = 0; i < mounted_filesystems.len(); i++)
    {
        if (mounted_filesystems[i].path.view() == (core::Str("/")))
        {
            VfsFileEndpoint *endpoint = new VfsFileEndpoint();

            log::log$("VFS: opening root filesystem endpoint: {}", mounted_filesystems[i].endpoint);
            endpoint->connection_to_fs = prot::FsFile::connect(mounted_filesystems[i].endpoint).unwrap();
            endpoint->server = core::move(prot::ManagedServer::create_server().unwrap());
            opened_file_endpoints.push(endpoint);
            return endpoint;
        }
    }

    return core::Result<VfsFileEndpoint *>::error("no root filesystem mounted");
}

void update_all_endpoints()
{
    for (auto &endpoint : opened_file_endpoints)
    {
        if (endpoint->server.connection_count() < 1)
        {

            endpoint->server.accept_connection();
        }

        auto received = endpoint->server.try_receive();
        if (!received.is_error())
        {
            auto msg = core::move(received.unwrap());

            switch (msg.received.data[0].data)
            {
            case prot::FS_OPEN_FILE:
            {
                IpcMessage reply = {};
                size_t filename_len = msg.received.len;
                core::WStr filename = {};
                for (size_t i = 0; i < filename_len; i++)
                {
                    filename.put(msg.received.raw_buffer[i]);
                }

                // TODO: check for path traversal mounted point

                auto file_res = endpoint->connection_to_fs.open_file(filename.view());
                if (file_res.is_error())
                {
                    reply.data[0].data = 0; // failure
                    reply.data[1].data = 0;
                    log::err$("VfsFileEndpoint: failed to open file {}: {}", filename.view(), file_res.error());
                    endpoint->server.reply(core::move(msg), reply);
                }
                else
                {
                    VfsFileEndpoint *nendpoint = new VfsFileEndpoint();

                    nendpoint->connection_to_fs = file_res.unwrap();
                    nendpoint->server = core::move(prot::ManagedServer::create_server().unwrap());
                    reply.data[0].data = 1; // success
                    reply.data[1].data = nendpoint->server.addr();


                    log::log$("VfsFileEndpoint: open file {}", filename.view());
                    endpoint->server.reply(core::move(msg), reply);
                    opened_file_endpoints.push(nendpoint);
                                       // early return because we loop over endpoints that we pushed
                    return;
                }

                break;
            }
                // else forward to filesystem

            case prot::FS_GET_INFO:
            case prot::FS_READ:
            case prot::FS_WRITE:
            case prot::FS_CLOSE:
            case prot::FS_LIST_DIR:
            {

                auto forward_res = endpoint->connection_to_fs.raw_client().send(msg.received, true);
                if (forward_res.is_error())
                {
                    log::err$("VfsFileEndpoint: failed to forward message to filesystem: {}", forward_res.error());
                }

                auto forward_handle = forward_res.unwrap();
                while (true)
                {
                    auto received_fs = endpoint->connection_to_fs.raw_client().receive_reply(forward_handle);
                    if (!received_fs.is_error())
                    {
                        auto fs_msg = core::move(received_fs.unwrap());
                        auto reply_res = endpoint->server.reply(core::move(msg), fs_msg);
                        if (reply_res.is_error())
                        {
                            log::err$("VfsFileEndpoint: failed to send reply back to client: {}", reply_res.error());
                        }
                        break;
                    }
                }
                break;
            }
            default:
            {
                log::log$("VfsFileEndpoint: unknown message type: {}", msg.received.data[0].data);
                break;
            }
            }
        }
    }
}
