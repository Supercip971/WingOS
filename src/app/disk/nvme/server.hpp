#pragma once

#include "libcore/str_writer.hpp"
#include "protocols/server_helper.hpp"

#include "app/disk/nvme/controller.hpp"
#include "iol/wingos/ipc.hpp"
#include "protocols/disk/disk.hpp"

class NvmeDiskConnection : public prot::ManagedServerConnectionHandler
{

public:
    NvmeController *controller;
    uint32_t uid;
    fc::WStr name;
    uint64_t device_id;

    NvmeDiskConnection(NvmeController *_controller, uint32_t _uid, fc::WStr const &_name, uint64_t _device_id)
        : controller(_controller), uid(_uid), name(_name.copy()), device_id(_device_id) {}

    virtual bool init() final { return true; }

    virtual fc::Result<void> call_received(IpcMessage &msg, fc::Optional<Wingos::IpcReplyObject> reply_obj [[maybe_unused]]) final
    {
        switch (msg.arg(0))
        {
        case prot::DISK_READ_SECTORS:
        {
            uint64_t lba = msg.arg(1);
            uint64_t size = msg.arg(2);

            uint64_t asset_handle = msg.asset(3);
            uint64_t mem_asset_off = msg.arg(4);
            // fmt::log$("Read sector off: {}", mem_asset_off);
            auto asset = Wingos::MemoryAsset::from_handle(asset_handle);

            uintptr_t buffer_ptr = asset.memory.start();
            auto &dev = controller->devices[device_id]; // for simplicity only first device

            if (size >= 512)
            {

                auto res = controller->read_write_ptr(&dev, false, lba, size / 512, (void *)(buffer_ptr + USERSPACE_VIRT_BASE + mem_asset_off), size);
            }
            else
            {
                auto res = controller->read_write_ptr(&dev, false, lba, size / 512, (void *)(buffer_ptr + USERSPACE_VIRT_BASE + mem_asset_off), size);

                if (res.is_error())
                {
                    fmt::err$("Failed to read sectors: {}", res.error());
                    break;
                }
            }

            IpcMessage reply = {};
            reply.arg(0, size); // number of sectors read
            reply.move_handle(1, asset.handle);

            ret(reply);
            break;
        }
        case prot::DISK_WRITE_SECTORS:
        {
            uint64_t lba = msg.arg(1);
            uint64_t size = msg.arg(2);
            uint64_t asset_handle = msg.asset(3);
            auto asset = Wingos::MemoryAsset::from_handle(asset_handle);

            uintptr_t buffer_ptr = asset.memory.start();
            auto &dev = controller->devices[device_id]; // for simplicity only first device
            auto res = controller->read_write_ptr(&dev, true, lba, size / 512, (void *)(buffer_ptr + USERSPACE_VIRT_BASE), size);

            if (res.is_error())
            {
                fmt::err$("Failed to write sectors: {}", res.error());
            }
            else
            {
                fmt::log$("Wrote sectors successful");
            }

            IpcMessage reply = {};
            reply.arg(0, size); // number of bytes written

            ret(reply).assert();
            break;
        }

        default:
            fmt::warn$("Unknown IPC command received: {}", msg.arg(0) | fmt::FMT_HEX);
            break;
        }
        return {};
    }

    virtual ~NvmeDiskConnection() = default;
};

class NvmeServer : public prot::ManagedServer
{

public:
    virtual fc::Result<prot::ManagedServerConnectionHandler *> on_connect(IpcMessage &initiator) final
    {
        (void)initiator;
        return "can't create connection by itself, connections must be made by forking or mounting";
    };

    virtual ~NvmeServer()
    {
    }
};
