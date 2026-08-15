#pragma once

#include "protocols/server_helper.hpp"

#include "app/fs/ext4/ext4.hpp"
#include "iol/wingos/ipc.hpp"
#include "libcore/ds/umap.hpp"
#include "libcore/fmt/log.hpp"
#include "libcore/optional.hpp"
#include "protocols/disk/disk.hpp"
#include "protocols/vfs/file.hpp"
#include "protocols/vfs/vfs.hpp"
#include "wingos-headers/ipc.h"

struct Ext4OpennedFileCtx
{
    uint64_t port;
    Ext4InodeRef inode;
    Ext4Filesystem *used_fs;
};

struct Ext4Ctx
{
    uint64_t core_endpoint_handle;
    fc::UMap<uint64_t, Ext4OpennedFileCtx> connections;
};

class Ext4InodeEndpoint : public prot::ManagedServerConnectionHandler
{
public:
    Ext4OpennedFileCtx *self;
    Ext4Ctx *fs;
    prot::ManagedServer *parent;
    Ext4InodeEndpoint(Ext4OpennedFileCtx *ctx, Ext4Ctx *_fs, prot::ManagedServer *_parent) : self(ctx), fs(_fs), parent(_parent) {};

    virtual bool init() { return true; }

    virtual void signal_disconnect(IpcMessage &msg)
    {
        (void)msg;
        delete self;
    }

    virtual fc::Result<void> call_received(IpcMessage &msg, [[maybe_unused]] fc::Optional<Wingos::IpcReplyObject> reply_obj)
    {
        switch (msg.arg(0))
        {

        case prot::FS_GET_INFO:
        {
            fmt::log$("ext4: get_info request for inode {}", self->inode.inode_id);

            fmt::log$("    indode size: {}", fc::copy(self->inode.inode.size_lo));
            fmt::log$("    inode type: {}", (int)self->inode.inode.file_type);
            IpcMessage reply_msg = {};
            reply_msg.arg(0, 1); // success
            reply_msg.arg(1, self->inode.inode.size_lo);
            reply_msg.arg(2, self->inode.inode.ctime);
            reply_msg.arg(3, self->inode.inode.mtime);
            reply_msg.arg(4, self->inode.inode.atime);
            reply_msg.arg(5, (uint16_t)(self->inode.inode.file_type));
            ret(reply_msg);
            return {};
        }
        case prot::FS_OPEN_FILE:
        {

            IpcMessage reply = {};
            fc::Str path;
            char path_buf[256];
            for (size_t i = 0; i < msg.len && i < 256; i++)
            {
                path_buf[i] = msg.raw_buffer[i];
            }
            path = fc::Str(path_buf, msg.len);
            fmt::log$("ext4: open file request for path: {}", path.view());

            auto file_res = self->used_fs->get_subdir(
                self->inode, path);

            fmt::log$("ext4: get_subdir result for path {}: {}", path.view(), file_res.is_error() ? file_res.error() : "success");
            if (file_res.is_error())
            {
                fmt::err$("ext4: failed to open file {}: {}", path.view(), file_res.error());
                reply.arg(0, 0); // failure
                reply.arg(1, 0);
                ret(reply);
            }
            else
            {
                auto file_inode = file_res.unwrap();
                fmt::log$("ext4: opened file {} with inode {}", path.view(), file_inode.inode_id);
                // create new endpoint for this file
                reply.arg(0, 1);

                Ext4InodeEndpoint *child = new Ext4InodeEndpoint(self, fs, parent);

                Ext4OpennedFileCtx *op = new Ext4OpennedFileCtx();
                op->used_fs = this->self->used_fs;
                op->inode = file_inode;
                op->port = child->get_port();
                child->self = op;

                child->fs = fs;

                auto forked = try$(parent->create_connection<Ext4InodeEndpoint>(child));

                reply.move_handle(1, forked.handle);
                fmt::log$("ext4: provided file endpoint {} for file {}", reply.asset(1), path.view());
                ret(reply);
                return {};
            }

            break;
        }
        case prot::FS_READ:
        {
            size_t offset = msg.arg(1);
            size_t len = msg.arg(2);

            // fmt::log$("ext4: read request for inode {} offset {} len {}", self->inode.inode_id, offset, len);

            auto mem_asset = Wingos::MemoryAsset::from_handle(msg.asset(3));
            auto r = this->self->used_fs->inode_read(
                self->inode,
                mem_asset,
                offset,
                len,
                0);

            if (r.is_error())
            {
                fmt::err$("ext4: failed to read inode {}: {}", self->inode.inode_id, r.error());
                while (true)
                {
                };
                IpcMessage reply = {};
                reply.arg(0); // failure
                ret(reply);
                break;
            }
            IpcMessage reply = {};
            reply.arg(0, 1); // success
            reply.arg(1, r.unwrap());
            reply.move_handle(2, mem_asset.handle);
            ret(reply);

            break;
        }
        default:
        {
            fmt::warn$("ext4: unknown message type received (endpoint): {}", msg.arg(0));
            break;
        }
        }

        return {};
    }

    virtual ~Ext4InodeEndpoint() {};
};

class Ext4CoreServer;

class Ext4Partition : public prot::ManagedServerConnectionHandler

{
public:
    fc::Optional<Wingos::IpcClient> client_to_be_given;
    Ext4Filesystem *fs_init_result;
    Ext4Ctx *ctx;
    prot::ManagedServer *parent;

    Ext4Partition() {};
    Ext4Partition(prot::ManagedServer *_parent) : parent(_parent) {};

    virtual bool init() { return true; }

    virtual void signal_disconnect(IpcMessage &msg)
    {
        // FIXME: flush
        (void)msg;
    };

    // return true on reply
    virtual fc::Result<void> call_received(IpcMessage &msg, fc::Optional<Wingos::IpcReplyObject> reply_obj)
    {
        switch (msg.arg(0))
        {
        case prot::VFS_DISK_ATTEMPT_INITIALIZE:
        {
            IpcServerHandle endpoint = msg.asset(1);
            size_t begin_lba = msg.arg(2);
            size_t end_lba = msg.arg(3);
            size_t part_id = msg.arg(4);

            fc::Str name;
            char name_buf[80];
            for (size_t i = 0; i < msg.len && i < 80; i++)
            {
                name_buf[i] = msg.raw_buffer[i];
            }
            name = fc::Str(name_buf, msg.len);

            fmt::log$("ext4: mount request for device {} (part id {})", name.view(), part_id);

            auto disk_conn_res = prot::DiskConnection::use_asset(endpoint);
            if (disk_conn_res.is_error())
            {
                fmt::err$("ext4: failed to connect to disk endpoint: {}", disk_conn_res.error());
                break;
            }

            auto disk_conn = disk_conn_res.unwrap();

            // check quickly if ext4 is present

            auto dfs = Ext4Filesystem::initialize(disk_conn, begin_lba, end_lba);
            if (dfs.is_error())
            {
                fmt::err$("ext4: failed to initialize ext4 filesystem on device {}: {}", name.view(), dfs.error());

                IpcMessage reply_msg = {};
                reply_msg.arg(0, 0); // fs endpoint 0 means failure
                reply(reply_msg, reply_obj);

                break;
            }

            auto dfs_res = dfs.unwrap();

            Ext4Partition *child = new Ext4Partition();
            child->fs_init_result = new Ext4Filesystem(dfs_res);
            child->ctx = ctx;
            child->parent = parent;

            auto forked = try$(parent->create_connection<Ext4Partition>(child));

            IpcMessage reply_msg = {};
            reply_msg.arg(0, 1); // success
            reply_msg.move_handle(1, forked.handle);

            // reply.data[1].data = ext4_file_roots[ext4_file_roots.len() - 1]->root_server.addr();
            (void)end_lba;
            fmt::log$("ext4: ext4 filesystem detected on device {}, mounting...", name.view());

            reply(reply_msg, reply_obj);
            break;
        }
        case prot::VFS_ACCESS_ROOT:
        {
            IpcMessage ret;

            ret.arg(0, 1);

            Ext4OpennedFileCtx op = {};
            op.used_fs = this->fs_init_result;
            op.inode = this->fs_init_result->read_inode(2).take();

            Ext4InodeEndpoint *ep = new Ext4InodeEndpoint(new Ext4OpennedFileCtx(op), this->ctx, this->parent);
            auto forked = try$(parent->create_connection<Ext4InodeEndpoint>(ep));

            ret.move_handle(1, forked.handle);

            reply(ret, reply_obj);
            break;
        }
        default:
        {
            fmt::warn$("ext4: unknown partition message received: {}", msg.arg(0));
            break;
        }
        }
        return {};
    }

    virtual ~Ext4Partition() {}
};

class Ext4CoreServer : public prot::ManagedServer
{
public:
    virtual fc::Result<prot::ManagedServerConnectionHandler *> on_connect(IpcMessage &initiator) final
    {
        switch (initiator.arg(0))
        {
        case prot::VFS_DISK_ATTEMPT_INITIALIZE:
            return new Ext4Partition(this);
            // case prot::VFS_ACCESS_PWD:
            // unimplemented
            break;

        default:
            fmt::err$("invalid open access for vfs: {}", initiator.arg(0));
            return "invalid open access for vfs";
        }
        return "invalid open access for vfs";
    }

    virtual fc::Result<void> after_receive() final
    {

        return {};
    }

    virtual ~Ext4CoreServer() {}
};
