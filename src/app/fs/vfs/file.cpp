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
    mnt.endpoint = prot::DiskFsImplementationConnection::connect(device_name).unwrap();
    mnt.path = core::move(mount_path);
    mounted_filesystems.push(core::move(mnt));

    return {};
}

core::Result<VfsFileEndpoint *> VfsFileEndpoint::open_root()
{
    log::log$("VfsFileEndpoint::open_root: searching for root filesystem, {} mounted", mounted_filesystems.len());
    for (size_t i = 0; i < mounted_filesystems.len(); i++)
    {
        log::log$("VfsFileEndpoint::open_root: checking mount {}: path={}", i, mounted_filesystems[i].path.view());
        if (mounted_filesystems[i].path.view() == (core::Str("/")))
        {
            VfsFileEndpoint *endpoint = new VfsFileEndpoint();


            auto root_endpoint = try$(mounted_filesystems[i].endpoint.create_root_endpoint());
            
            log::log$("VFS: opening root filesystem endpoint: {}", root_endpoint);
            auto connect_res = prot::FsFile::connect(
                root_endpoint);
            if (connect_res.is_error())
            {
                log::err$("VFS: failed to connect to root filesystem: {}", connect_res.error());
                delete endpoint;
                return connect_res.error();
            }
            endpoint->connection_to_fs = core::move(connect_res.unwrap());

            log::log$("VFS: connected to root fs, client handle: {}", endpoint->connection_to_fs.raw_client().handle);

            auto server_res = prot::ManagedServer::create_server();
            if (server_res.is_error())
            {
                log::err$("VFS: failed to create server for root endpoint: {}", server_res.error());
                endpoint->connection_to_fs.close();
                delete endpoint;
                return server_res.error();
            }
            endpoint->server = core::move(server_res.unwrap());

            log::log$("VFS: created root endpoint server with addr: {}", endpoint->server.addr());
            opened_file_endpoints.push(endpoint);

            return endpoint;
        }
    }

    log::err$("VfsFileEndpoint::open_root: no root filesystem mounted");
    return core::Result<VfsFileEndpoint *>::error("no root filesystem mounted");
}

void close_endpoint(VfsFileEndpoint *endpoint)
{

    // don't forward if endpoint is root

    // now disconnect to the client
    for (size_t i = 0; i < opened_file_endpoints.len(); i++)
    {
        if (opened_file_endpoints[i] == endpoint)
        {
            opened_file_endpoints.pop(i);

            endpoint->connection_to_fs.close().unwrap();
            endpoint->server.close();
            delete endpoint;
            return;
        }
    }

    log::err$("VfsFileEndpoint: attempted to close unknown endpoint");
}

void update_all_endpoints()
{
    for (auto endpoint : opened_file_endpoints)
    {
        // Always try to accept new connections (there may be multiple pending)
        while (endpoint->server.accept_connection())
        {
            // Continue accepting until no more connections are pending
        }

        auto received = endpoint->server.try_receive();

        if (!received.is_error())
        {
            auto msg = core::move(received.unwrap());

            if (msg.received.flags & IPC_MESSAGE_FLAG_DISCONNECT)
            {

                close_endpoint(endpoint);

                return;

            }

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

                    nendpoint->connection_to_fs = core::move(file_res.unwrap());
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
            case prot::FS_CLOSE:
            {
                // Note: FS_CLOSE is no longer sent by FsFile::close() - it just disconnects.
                // This code path should not be reached anymore, but keeping it for safety.
                // The actual cleanup happens when IPC_MESSAGE_FLAG_DISCONNECT is received above.
                log::err$("VfsFileEndpoint: received explicit FS_CLOSE (deprecated path) - THIS IS A BUG!");
                log::err$("  message_id: {}", msg.received.message_id);
                log::err$("  flags: {}", msg.received.flags);

                int a= msg.received.data[0].is_asset;

                int b= msg.received.data[1].is_asset;
                log::err$("  data[0]: is_asset={}, data={}", a, msg.received.data[0].data);
                log::err$("  data[1]: is_asset={}, data={}", b, msg.received.data[1].data);
                log::err$("  len: {}", msg.received.len);
                log::err$("  connection ptr: {}", (uint64_t)msg.connection);
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
