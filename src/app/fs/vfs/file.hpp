#pragma once

#include "libcore/str_writer.hpp"
#include "protocols/server_helper.hpp"

#include "app/fs/vfs/ctx.hpp"
#include "iol/wingos/ipc.hpp"
#include "libcore/optional.hpp"
#include "libcore/result.hpp"
#include "protocols/vfs/file.hpp"
#include "wingos-headers/ipc.h"
fc::Result<void> mount_fs(IpcServerHandle device_name, fc::WStr &&mount_path);

// app -> server -> connection to fs -> ext2
class VfsConnectionFile : public prot::ManagedServerConnectionHandler
{
    VfsServerCtx &ctx;

public:
    prot::FsFile *connection_to_fs = {};

    VfsConnectionFile(VfsServerCtx &_ctx) : ctx(_ctx) {}

    virtual bool init() { return true; }

    static fc::Result<Wingos::IpcClient> open_root(VfsServerCtx &ctx);

    virtual void signal_disconnect(IpcMessage &msg)
    {
        (void)msg;
        connection_to_fs->close();
    };

    // return true on reply
    virtual fc::Result<void> call_received(IpcMessage &msg, fc::Optional<Wingos::IpcReplyObject> reply_obj)
    {
        switch (msg.arg(0))
        {
        case prot::FS_OPEN_FILE:
        {
            IpcMessage reply_msg = {};
            size_t filename_len = msg.len;
            fc::WStr filename = {};
            for (size_t i = 0; i < filename_len; i++)
            {
                filename.put(msg.raw_buffer[i]);
            }

            // TODO: check for path traversal mounted point

            auto raw_file = new prot::FsFile(try$(connection_to_fs->open_file(filename.view())));
            // connect to ourselves

            VfsConnectionFile *forked = new VfsConnectionFile(ctx);
            forked->connection_to_fs = raw_file;

            auto client = try$(ctx.root_vfs->create_connection<VfsConnectionFile>(forked));

            reply_msg.arg(0, 1);
            reply_msg.move_handle(1, client.handle);

            fmt::log$("VfsFileEndpoint: open file {}", filename.view());

            reply(reply_msg, reply_obj);

            // early return because we loop over endpoints that we pushed
            return {};

            break;
        }
            // else forward to filesystem

        case prot::FS_GET_INFO:
        case prot::FS_READ:
        case prot::FS_WRITE:
        case prot::FS_LIST_DIR:
        {
            auto forward_res = connection_to_fs->raw_client().call(msg);
            if (forward_res.is_error())
            {
                fmt::err$("VfsFileEndpoint: failed to forward message to filesystem: {}", forward_res.error());
            }

            try$(reply(msg, reply_obj));

            break;
        }

        default:
        {
            fmt::log$("VfsFileEndpoint: unknown message type: {}", msg.arg(0));
            break;
        }
        }
        return {};
    }

    virtual ~VfsConnectionFile() {}
};
