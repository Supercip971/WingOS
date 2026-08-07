#pragma once

#include "libcore/fmt/fmt_str.hpp"
#include "protocols/server_helper.hpp"

#include "app/fs/vfs/ctx.hpp"
#include "app/fs/vfs/file.hpp"
#include "fs/gpt/gpt.hpp"
#include "protocols/disk/disk.hpp"
#include "protocols/vfs/vfs.hpp"

class VfsServerAdministration : public prot::ManagedServerConnectionHandler
{

    VfsServerCtx &ctx;

public:
    VfsServerAdministration(VfsServerCtx &_ctx) : ctx(_ctx) {}

    bool init() final
    {
        return true;
    }

    virtual fc::Result<void> call_received(IpcMessage &msg, fc::Optional<Wingos::IpcReplyObject> reply_obj) final
    {
        switch (msg.arg(0))
        {
        case prot::VFS_REGISTER_FS:
        {

            RegisteredFs filesystem{};
            for (size_t i = 0; i < msg.len; i++)
            {
                filesystem.name[i] = msg.raw_buffer[i];
            }
            filesystem.endpoint_to_fs = prot::DiskFsManagerConnection::connect_by_addr(msg.arg(1)).unwrap();

            ctx.registered_fs.push(filesystem);

            fmt::log$("(server) registered filesystem: {} with endpoint: {}", fc::Str(filesystem.name), msg.arg(1));

            ctx.dirty_mount = true;
            ack(reply_obj);
            return {};
        }
        case prot::VFS_ACCESS_ROOT:
        {

            if (ctx.mounted_devices_count == 0)
            {
                return "no mounted devices";
            }

            auto root_endpoint = VfsConnectionFile::open_root(this->ctx);
            if (root_endpoint.is_error())
            {
                return root_endpoint.error();
            }

            IpcMessage reply_msg{};

            reply_msg.arg(0, 1); // success
            reply_msg.move_handle(1, root_endpoint.unwrap()->client_to_be_given->handle);
            reply(reply_msg, reply_obj);
            return {};
        }
        case prot::VFS_REGISTER_DISK:
        {

            RegisteredDevice device{};
            for (size_t i = 0; i < 80 && msg.raw_buffer[i] != 0; i++)
            {
                device.name[i] = msg.raw_buffer[i];
            }
            device.connection_handle = try$(prot::DiskConnection::use_asset(msg.asset(1)));
            device.has_partitions = false;

            fmt::log$("(server) registered device: {}", device.name);

            fc::Str v = fc::Str(device.name);
            auto v2_res = Wingos::parse_gpt(v);
            auto v2 = v2_res.take();

            size_t part_id = 0;
            for (auto &entry : v2.entries)
            {
                RegisteredDevicePartition part{};
                part.id = part_id++;
                part.disk = device.connection_handle;
                fc::WStr part_name = fmt::format_str("{}-{}", device.name, part.id).take();
                part.part_dev_name = fc::WStr::copy(part_name.view());
                part.part_name = fc::WStr::copy(entry.name.view());
                part.has_fs = false;
                part.begin = entry.entry->lba_start;
                part.end = entry.entry->lba_end;

                fmt::log$("(server) detected partition: {} -> (LBA {} - {})", part.part_name.view(), part.part_dev_name.view(), part.begin, part.end);

                device.partitions.push(std::move(part));
            }

            ctx.registered_services.push(std::move(device));

            ctx.dirty_mount = true;

            ack(reply_obj);
            break;
        }
        }
        return {};
    }
};
