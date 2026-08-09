
#include "protocols/server_helper.hpp"

#include "app/disk/nvme/controller.hpp"
#include "app/disk/nvme/server.hpp"
uint64_t device_uid;

int main(int, char **)
{

    fmt::log$("hello world from nvme!");
    Wingos::dev::PciController pci_controller;
    pci_controller.scan_bus(0);

    device_uid = 0;

    fc::Vec<NvmeController> disks = {};
    fc::Vec<ControllerEndpoint> endpoints = {};
    for (auto &dev : pci_controller.devices)
    {
        if (dev.class_code() == 0x01 && dev.subclass() == 0x08) // storage controller, NVMe
        {
            fmt::log$("Found NVMe device: Bus {}, Device {}, Function {}, Vendor ID: {}, Device ID: {}",
                      dev.bus, dev.device, dev.function,
                      dev.vendor_id() | fmt::FMT_HEX, dev.device_id() | fmt::FMT_HEX);
            auto disk = NvmeController::setup(dev);
            if (!disk.is_error())
            {
                disks.push(disk.take());

                auto mapped = Wingos::Space::self().allocate_memory(4096, false);
                fmt::log$("Allocated memory at: {}", (uintptr_t)mapped.ptr() | fmt::FMT_HEX);

                disks[0].read_write_ptr(&disks[0].devices[0], false, 0, 8, mapped.ptr(), 4096);

                fmt::log$("NVMe worked !");
            }
            else
            {
                fmt::err$("Failed to setup NVMe driver: {}", disk.error());
            }
        }
    }

    auto v = prot::VfsConnection::connect();

    if (v.is_error())
    {
        fmt::err$("Failed to connect to VFS: {}", v.error());
        return 1;
    }

    auto vfs = v.take();

    auto nvme = prot::ManagedServer::create_registered_server<NvmeServer>("nvme@root").take();

    for (auto &disk : disks)
    {
        for (auto &dev : disk.devices)
        {

            auto fmt_str_res = fmt::format_str("nvme{}", (int)dev.sys_id).take();

            auto disk_conn =
                new NvmeDiskConnection(&disk, dev.sys_id, fmt_str_res.copy(), dev.sys_id);
            auto conn = nvme->create_connection<NvmeDiskConnection>(disk_conn)
                            .take();

            fmt::log$("Registered endpoint {} with uid {}", disk_conn->name.view(), disk_conn->uid);

            vfs.register_device(disk_conn->name.view(), conn.handle).assert();
        }
    }

    fmt::log$("Entering main NVMe IPC loop");
}
