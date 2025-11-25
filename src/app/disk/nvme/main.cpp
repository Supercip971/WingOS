
#include <string.h>

#include "libcore/fmt/fmt_str.hpp"
#include "libcore/str_writer.hpp"

#include "dev/pci/pci.hpp"
#include "iol/wingos/asset.hpp"
#include "iol/wingos/space.hpp"
#include "iol/wingos/syscalls.h"
#include "libcore/fmt/flags.hpp"
#include "libcore/fmt/log.hpp"
#include "libcore/type/trait.hpp"
#include "math/align.hpp"
#include "mcx/mcx.hpp"
#include "protocols/disk/disk.hpp"
#include "protocols/vfs/vfs.hpp"
#include "spec.hpp"
#include "wingos-headers/asset.h"
#include "wingos-headers/syscalls.h"
#include "protocols/server_helper.hpp"

uint64_t device_uid;

class NvmeController
{

    ControllerCap cap;
    NvmeIdentifyController *identify_controller;
    uint64_t stride;
    uint64_t queue_slots;
    uint64_t max_transfer;

    uint32_t *nsids;
    void *base_addr;

    // uint64_t uid;

    template <typename T>
    struct Queue
    {
        uint64_t base_addr;
        uint64_t physical_addr;
        uint64_t count;
        bool is_completion;
        uint64_t tail;
        size_t page_size;

        bool will_loop()
        {
            return (tail + 1) >= count;
        }
        core::Result<T *> allocate()
        {
            T *res = (T *)(base_addr + tail * sizeof(T));

            tail += 1;

            if (tail >= count)
            {
                tail = 0;
                return allocate(); // circular  buffer
            }

            return res;
        }

        static core::Result<Queue<T>> create(size_t count)
        {
            Queue<T> queue = {};

            size_t len = math::alignUp(count * sizeof(T), 4096ul);

            queue.count = math::alignDown(len, sizeof(T)) / sizeof(T);
            log::log$("Creating queue of {} entries ({} bytes)", queue.count, len);
            queue.is_completion = core::IsSame<T, CompletionQueueEntry>;
            queue.tail = 0;

            auto memory = Wingos::Space::self().allocate_physical_memory(len, false);
            if (memory.handle == 0)
            {
                return core::Result<Queue<T>>::error("failed to allocate memory for queue");
            }
            auto mapped = Wingos::Space::self().map_memory(memory, ASSET_MAPPING_FLAG_WRITE | ASSET_MAPPING_FLAG_EXECUTE);
            queue.base_addr = (uint64_t)mapped.ptr();
            queue.physical_addr = (uint64_t)mapped.ptr() - USERSPACE_VIRT_BASE;
            queue.page_size = len;
            memset((void *)queue.base_addr, 0, len);

            return queue;
        };

        core::Result<void> release()
        {
            Wingos::Space::self().release_memory((void*)this->base_addr, this->page_size);
            return {};
        }
    };

    template <typename SubmitT = NvmeCmd, typename CompleteT = CompletionQueueEntry>
    struct Queues
    {

        Queue<SubmitT> command_queue;
        Queue<CompleteT> complete_queue;

        uint64_t id;
        uint64_t cid;
        uint64_t phase;
        uint64_t *pgs_reg; // indirect memory page size register
    };

    template <typename SubmitT = NvmeCmd, typename CompleteT = CompletionQueueEntry>
    core::Result<Queues<SubmitT, CompleteT>> create_queues(size_t slots, uint64_t id)
    {

        Queues<SubmitT, CompleteT> queues;

        auto cmd_queue = try$(Queue<SubmitT>().create(slots));
        auto cmp_queue = try$(Queue<CompleteT>().create(slots));

        queues.command_queue = cmd_queue;
        queues.complete_queue = cmp_queue;

        queues.phase = 1;
        queues.id = id;
        queues.cid = 0;

        return queues;
    }
    // Queue<SubmissionQueueEntry> io_command_queue;
    // Queue<CompletionQueueEntry> io_complete_queue;

    Queues<> admin_queues;

    struct NvmeDevice
    {
        Queues<> io_queues;
        uint32_t nsid;
        uint64_t sys_id;
        core::Str name;
        size_t max_phys_rpgs;
        size_t lba_size;
        NvmeIdentifyNamespace *identify_namespace;
    };

    Wingos::VirtualMemoryAsset mapped_nvme_addr_space;

    void *remap_nvme_addr(uint64_t base, uint64_t len)
    {
        base = math::alignDown(base, 4096ul);
        len = math::alignUp(len + base, 4096ul) - base;

        mapped_nvme_addr_space = Wingos::Space::self().map_physical_memory(base, len, ASSET_MAPPING_FLAG_WRITE | ASSET_MAPPING_FLAG_EXECUTE);

        return mapped_nvme_addr_space.ptr();
    }
    uint32_t read(int reg)
    {
        void *base = mapped_nvme_addr_space.ptr();

        uint32_t volatile *offseted = reinterpret_cast<uint32_t volatile *>((uintptr_t)base + reg);

        return *offseted;
    }
    uint64_t read64(int reg)
    {
        void *base = mapped_nvme_addr_space.ptr();

        uint64_t volatile *offseted = reinterpret_cast<uint64_t volatile *>((uintptr_t)base + reg);

        return *offseted;
    }

    void write(int reg, uint32_t val)
    {
        void *base = mapped_nvme_addr_space.ptr();

        uint32_t volatile *offseted = reinterpret_cast<uint32_t volatile *>((uintptr_t)base + reg);

        *offseted = val;
    }
    void write64(int reg, uint64_t val)
    {
        void *base = mapped_nvme_addr_space.ptr();

        uint64_t volatile *offseted = reinterpret_cast<uint64_t volatile *>((uintptr_t)base + reg);

        *offseted = val;
    }

    core::Result<void> nvme_await_submit(NvmeCmd const *cmd, Queues<> &queues)
    {
        if (queues.complete_queue.will_loop())
        {
            // need to toggle phase
            queues.phase = !queues.phase;
        }

        NvmeCmd *submission = try$(queues.command_queue.allocate());
        CompletionQueueEntry *completion = try$(queues.complete_queue.allocate());

        *submission = *cmd; // copy
        submission->header.cid = queues.cid++;

        // submit command

        write(NVME_QUEUE_TAIL_DOORBELL_BASE + (2 * queues.id * (4 << stride)), queues.command_queue.tail);

        uint16_t status = 0;

        while (true)
        {
            if ((completion->phase_bit) == queues.phase)
            {
                status = completion->status;
                break;
            }
            asm volatile("pause");
        }

        write(NVME_QUEUE_TAIL_DOORBELL_BASE + (2 * queues.id + 1) * (4 << stride), queues.complete_queue.tail);

        if (status != 0)
        {
            log::err$("NVMe command failed with status: {}", status | fmt::FMT_HEX);
            return "nvme command failed";
        }

        return {};
    }

    core::Result<void> identify()
    {
        size_t len = math::alignUp(sizeof(NvmeIdentifyController), 4096ul);
        auto cmd = NvmeCmd{};

        auto mem = Wingos::Space::self().allocate_memory(len, false);

        cmd.header.opcode = NVME_AOP_IDENTIFY;
        cmd.nsid = 0;
        cmd.Identify.cns = 1; // identify controller
        cmd.prp1 = (uintptr_t)mem.ptr() - USERSPACE_VIRT_BASE;

        len -= 0x1000;
        if (len > 0)
        {
            cmd.prp2 = cmd.prp1 + 0x1000;
        }
        else
        {
            cmd.prp2 = 0;
        }

        try$(nvme_await_submit(&cmd, admin_queues));

        identify_controller = (NvmeIdentifyController *)mem.ptr();

        return {};
    };

    core::Result<void> nvme_set_queue_count(uint16_t count)
    {
        if (count == 0 || count > cap.max_queue_entries)
        {
            return "invalid queue count";
        }

        NvmeCmd cmd = {};
        cmd.header.opcode = NVME_AOP_SET_FEATURE;
        cmd.prp1 = 0;
        cmd.Feature.fid = 0x07;
        cmd.Feature.dword11 = (count - 1) | ((count - 1) << 16);

        return nvme_await_submit(&cmd, admin_queues);
    }
    core::Result<void> nvme_create_queues(NvmeDevice *dev, uint16_t queue_id)
    {

        dev->io_queues = try$(create_queues(queue_slots, queue_id));

        NvmeCmd cmd_completion = {};
        cmd_completion.header.opcode = NVME_AOP_CREATE_COMPLETION_QUEUE;

        cmd_completion.CreateIoCompletionQueue.qid = queue_id;
        cmd_completion.CreateIoCompletionQueue.qsize = dev->io_queues.complete_queue.count - 1;
        cmd_completion.CreateIoCompletionQueue.pc = 1; // physically contiguous
        cmd_completion.prp1 = (dev->io_queues.complete_queue.physical_addr);
        try$(nvme_await_submit(&cmd_completion, admin_queues));

        NvmeCmd cmd_submission = {};
        cmd_submission.header.opcode = NVME_AOP_CREATE_SUBMISSION_QUEUE;
        cmd_submission.CreateIoSubmissionQueue.qid = queue_id;
        cmd_submission.CreateIoSubmissionQueue.qsize = dev->io_queues.command_queue.count - 1;
        cmd_submission.CreateIoSubmissionQueue.pc = 1; // physically contiguous
        cmd_submission.CreateIoSubmissionQueue.qprio = 2;
        cmd_submission.CreateIoSubmissionQueue.cqid = queue_id;
        cmd_submission.prp1 = (dev->io_queues.command_queue.physical_addr);
        try$(nvme_await_submit(&cmd_submission, admin_queues));

        return {};
    }

    core::Result<void> nvme_register_device_namespace(size_t nsid)
    {
        NvmeDevice device = {};
        device.nsid = nsid;
        device.sys_id = device_uid++;

        device.identify_namespace = (NvmeIdentifyNamespace *)Wingos::Space::self().allocate_memory(4096, false).ptr();

        NvmeCmd idns_cmd = {};
        idns_cmd.header.opcode = NVME_AOP_IDENTIFY; // identify
        idns_cmd.nsid = nsid;
        idns_cmd.Identify.cns = 0; // identify namespace
        idns_cmd.prp1 = (uintptr_t)device.identify_namespace - USERSPACE_VIRT_BASE;
        idns_cmd.prp2 = 0;
        try$(nvme_await_submit(&idns_cmd, admin_queues));

        log::log$(" - Namespace {}: {} blocks of size {}", nsid, core::copy(device.identify_namespace->nsze), 1 << (device.identify_namespace->lbaf[device.identify_namespace->flbas & 0xf].lbads));

        uint64_t flba = device.identify_namespace->flbas & 0xf;
        uint64_t lba_shift = (device.identify_namespace->lbaf[flba].lbads);
        uint64_t max_lba = (this->max_transfer) / (1 << lba_shift);

        log::log$("   - max transfer size: {} bytes ({} blocks)", this->max_transfer | fmt::FMT_HEX, max_lba | fmt::FMT_HEX);

        device.max_phys_rpgs = max_lba / (1 << (12 + cap.memory_page_size_minimum));

        device.lba_size = 1 << lba_shift;

        try$(nvme_create_queues(&device, nsid)); // use nsid as queue id for simplicity

        this->devices.push(device);

        return {};
    }

public:
    core::Vec<NvmeDevice> devices;
    core::Result<void> read_write_ptr(NvmeDevice *dev, bool is_write, uint64_t lba, uint16_t nlb, void *buffer, size_t buffer_len)
    {
        if (buffer_len == 0 || buffer == nullptr)
        {
            return "invalid buffer";
        }

        size_t len = math::alignUp(buffer_len, 4096ul);

        if ((nlb * dev->lba_size) > len)
        {
            log::log$("buffer too small: {} bytes for {} blocks of size {}", len, nlb, dev->lba_size);
            return "buffer too small for requested nlb";
        }

        if (nlb * dev->lba_size > max_transfer)
        {
            return "requested nlb too large for max transfer";
        }

        bool use_prp = false;
        if (nlb * dev->lba_size > 4096)
        {
            use_prp = true;
            if (nlb * dev->lba_size > (4096 * 2))
            {
                return "prp list not supported yet";
            }
        }

        NvmeCmd cmd = {};
        cmd.header.opcode = is_write ? NVME_OP_WRITE : NVME_OP_READ;
        cmd.nsid = dev->nsid;
        cmd.ReadWrite.lba = lba;
        cmd.ReadWrite.nlb = nlb - 1; // 0's based

        if (use_prp)
        {
            cmd.prp2 = (uintptr_t)buffer - USERSPACE_VIRT_BASE + 0x1000;
        }
        else
        {
            cmd.prp1 = (uintptr_t)buffer - USERSPACE_VIRT_BASE;
        }

        return nvme_await_submit(&cmd, dev->io_queues);
    }

    static core::Result<NvmeController> setup(Wingos::dev::PciDevice &dev)
    {
        NvmeController driver;
        uint64_t device_addr = dev.get_bar64(Wingos::dev::pci_reg_dev_BAR0); // Get the BAR0 address

        auto len = dev.get_bar64_size(Wingos::dev::pci_reg_dev_BAR0);
        log::log$("Setting up NVMe disk at address: {} - {}", device_addr | fmt::FMT_HEX, (device_addr + len - 1) | fmt::FMT_HEX);

        driver.base_addr = driver.remap_nvme_addr(device_addr, len);

        driver.stride = (device_addr >> 12) & 0xf;

        log::log$("remapped nvme at: {}", ((uint64_t)driver.base_addr) | fmt::FMT_HEX);

        driver.cap.raw_value_0 = driver.read(NVME_CAP);
        driver.cap.raw_value_1 = driver.read(NVME_CAP + sizeof(uint32_t));

        NvmeConfig cfg = {};
        cfg.raw_value = driver.read(NVME_CONTROLLER_CONFIG);

        if (cfg.en)
        {
            cfg.en = 0;

            log::log$("Disabling NVMe controller to reset it...");
            driver.write(NVME_CONTROLLER_CONFIG, cfg.raw_value);
        }
        while (driver.read(NVME_CONTROLLER_STATUS) & 1)
        {
            asm volatile("pause");
        }
        driver.stride = (driver.cap.stride);
        driver.queue_slots = driver.cap.max_queue_entries;

        log::log$("NVMe stride is: {}", driver.stride | fmt::FMT_HEX);
        log::log$("NVMe queue slots: {}", driver.queue_slots);

        driver.admin_queues = try$(driver.create_queues(driver.queue_slots, 0));

        dump_controller_cap(driver.cap);

        driver.write(NVME_ADMIN_QUEUE_ATTRIBUTE, ((driver.queue_slots - 1) << 16) | (driver.queue_slots - 1));

        driver.write64(NVME_ADMIN_QUEUE_SUBMIT, (driver.admin_queues.command_queue.physical_addr));
        driver.write64(NVME_ADMIN_QUEUE_COMPLETE, (driver.admin_queues.complete_queue.physical_addr));

        NvmeConfig new_cfg = {};
        new_cfg.en = 1;
        new_cfg.iocqes = 4; // 16 bytes
        new_cfg.iosqes = 6; // 64 bytes
        new_cfg.shn = 0;    // no shutdown notification
        new_cfg.ams = 0;    // 0 = round robin
        new_cfg.css = 0;    // 0 = nvm command set

        driver.write(NVME_CONTROLLER_CONFIG, new_cfg.raw_value);

        while (true)
        {
            uint32_t status = driver.read(NVME_CONTROLLER_STATUS);
            if ((status & 1) == 1)
            {
                break;
            }

            if (status & (1 << 1))
            {
                log::err$("NVMe controller failed to start, fatal error: {}", status | fmt::FMT_HEX);
                return core::Result<NvmeController>::error("nvme controller failed to start");
            }
            asm volatile("pause");
        }

        log::log$("NVMe controller started successfully!");

        try$(driver.identify());

        log::log$("NVMe Identify Controller:");
        log::log$("- Vendor ID       : {}", core::copy(driver.identify_controller->vid) | fmt::FMT_HEX);
        log::log$("- Subsystem VID   : {}", core::copy(driver.identify_controller->ssvid) | fmt::FMT_HEX);
        log::log$("- Serial Number   : {}", core::Str(driver.identify_controller->sn, 20));
        log::log$("- Model Number    : {}", core::Str(driver.identify_controller->mn, 40));
        log::log$("- Firmware Revision : {}", core::Str(driver.identify_controller->fr, 8));
        log::log$("- Max Data Transfer Size : {}", 1 << driver.identify_controller->mdts);
        log::log$("- Number of Namespaces : {}", core::copy(driver.identify_controller->nn));
        log::log$("- Submission Queue Entry Size : {}", 1 << driver.identify_controller->sqes);
        log::log$("- Completion Queue Entry Size : {}", 1 << driver.identify_controller->cqes);
        log::log$("- Max Outstanding Commands : {}", driver.identify_controller->maxcmd + 1);

        size_t nsids_len = driver.identify_controller->nn * sizeof(uint32_t);
        nsids_len = math::alignUp(nsids_len, 4096ul);
        driver.nsids = (uint32_t *)Wingos::Space::self().allocate_memory(nsids_len, false).ptr();
        memset(driver.nsids, 0, nsids_len);

        // identify namespaces
        NvmeCmd idns_cmd = {};
        idns_cmd.header.opcode = NVME_AOP_IDENTIFY; // identify
        idns_cmd.nsid = 0;
        idns_cmd.Identify.cns = 2; // identify namespace
        idns_cmd.prp1 = (uintptr_t)driver.nsids - USERSPACE_VIRT_BASE;
        idns_cmd.prp2 = 0;
        try$(driver.nvme_await_submit(&idns_cmd, driver.admin_queues));

        size_t shift = 12 + driver.cap.memory_page_size_minimum;

        if (driver.identify_controller->mdts)
        {

            driver.max_transfer = 1 << (driver.identify_controller->mdts + shift);
        }
        else
        {
            driver.max_transfer = 1 << 20; // 1 MB
        }

        log::log$("Found {} namespaces:", core::copy(driver.identify_controller->nn));

        try$(driver.nvme_set_queue_count(4));
        for (uint32_t i = 0; i < driver.identify_controller->nn; i++)
        {
            if (driver.nsids[i] == 0)
                continue;

            if (driver.nsids[i] > driver.identify_controller->mnan && driver.identify_controller->mnan != 0)
            {
                log::err$("Namespace ID {} is greater than maximum number of namespaces {}", core::copy(driver.nsids[i]), core::copy(driver.identify_controller->mnan));
                continue;
            }
            log::log$("- Namespace ID: {}", core::copy(driver.nsids[i]));
            try$(driver.nvme_register_device_namespace(driver.nsids[i]));
        }

        return driver;
    }
};

struct ControllerEndpoint
{
    uint32_t uid;
    core::WStr name;
    NvmeController *controller;
    uint64_t device_id;

    prot::ManagedServer server;
};

int main(int, char**)
{

    log::log$("hello world from nvme!");
    Wingos::dev::PciController pci_controller;
    pci_controller.scan_bus(0);

    device_uid = 0;

    core::Vec<NvmeController> disks = {};
    core::Vec<ControllerEndpoint> endpoints = {};
    for (auto &dev : pci_controller.devices)
    {
        if (dev.class_code() == 0x01 && dev.subclass() == 0x08) // storage controller, NVMe
        {
            log::log$("Found NVMe device: Bus {}, Device {}, Function {}, Vendor ID: {}, Device ID: {}",
                      dev.bus, dev.device, dev.function,
                      dev.vendor_id() | fmt::FMT_HEX, dev.device_id() | fmt::FMT_HEX);
            auto disk = NvmeController::setup(dev);
            if (!disk.is_error())
            {
                disks.push(disk.take());

                auto mapped = Wingos::Space::self().allocate_memory(4096, false);
                log::log$("Allocated memory at: {}", (uintptr_t)mapped.ptr() | fmt::FMT_HEX);

                disks[0].read_write_ptr(&disks[0].devices[0], false, 0, 8, mapped.ptr(), 4096);

                log::log$("NVMe worked !");
            }
            else
            {
                log::err$("Failed to setup NVMe driver: {}", disk.error());
            }
        }
    }

    auto v = prot::VfsConnection::connect();

    if (v.is_error())
    {
        log::err$("Failed to connect to VFS: {}", v.error());
        return 1;
    }

    auto vfs = v.take();

    for (auto &disk : disks)
    {
        for (auto &dev : disk.devices)
        {
            ControllerEndpoint ep = {};
            ep.controller = &disk;
            ep.uid = dev.sys_id;

            auto fmt_str_res = fmt::format_str("nvme{}", (int)dev.sys_id);
            ep.name = (fmt_str_res.take());


            auto srv = prot::ManagedServer::create_registered_server(ep.name.view(), 1, 0);
            ep.server = srv.take();

            log::log$("Registered endpoint {} with uid {} (ip: {})", ep.name.view(), ep.uid, ep.server.addr());

            vfs.register_device(ep.name.view(), ep.server.addr()).assert();
            endpoints.push(core::move(ep));
        }
    }

    log::log$("Entering main NVMe IPC loop");

    while (true)
    {
        for (auto &ep : endpoints)
        {
            ep.server.accept_connection();

            auto received = ep.server.try_receive();
            if (received.is_error())
            {
                continue;
            }

            auto msg = (received.take());

            switch (msg.received.data[0].data)
            {
            case prot::DISK_READ_SECTORS:
            {
                uint64_t lba = msg.received.data[1].data;
                uint16_t size = msg.received.data[2].data;
                uint64_t asset_handle = msg.received.data[3].asset_handle;
                auto asset = Wingos::MemoryAsset::from_handle(asset_handle);

                uintptr_t buffer_ptr = asset.memory.start();
                auto &dev = ep.controller->devices[ep.device_id]; // for simplicity only first device

                if (size >= 512)
                {
                    for (size_t i = 0; i < size; i += 512)
                    {
               
                        auto res = ep.controller->read_write_ptr(&dev, false, lba + (i / 512), 1, (void *)(buffer_ptr + USERSPACE_VIRT_BASE + i  ), 512);

                        if (res.is_error())
                        {
                            log::err$("Failed to read sectors: {}", res.error());
                            break;
                        }
                    }
                }
                else 
                {
                    auto res = ep.controller->read_write_ptr(&dev, false, lba, size/512, (void *)(buffer_ptr + USERSPACE_VIRT_BASE), size);

                    if (res.is_error())
                    {
                        log::err$("Failed to read sectors: {}", res.error());
                        break;
                    }
                }

                IpcMessage reply = {};
                reply.data[0].data = size; // number of sectors read
                reply.data[0].is_asset = false;
                reply.data[1].is_asset = true;
                reply.data[1].asset_handle = asset.handle;

                ep.server.reply(core::move(msg), reply).assert();
                break;
            }
            case prot::DISK_WRITE_SECTORS:
            {
                uint64_t lba = msg.received.data[1].data;
                uint16_t size = msg.received.data[2].data;
                uint64_t asset_handle = msg.received.data[3].asset_handle;
                auto asset = Wingos::MemoryAsset::from_handle(asset_handle);

                uintptr_t buffer_ptr = asset.memory.start();
                auto &dev = ep.controller->devices[ep.device_id]; // for simplicity only first device
                auto res = ep.controller->read_write_ptr(&dev, true, lba, size, (void *)(buffer_ptr + USERSPACE_VIRT_BASE), size);

                if (res.is_error())
                {
                    log::err$("Failed to write sectors: {}", res.error());
                }
                else
                {
                    log::log$("Wrote sectors successful");
                }

                IpcMessage reply = {};
                reply.data[0].data = size; // number of sectors written
                reply.data[0].is_asset = false;

                ep.server.reply(core::move(msg), reply).assert();
                break;
            }

            default:
                log::warn$("Unknown IPC command received: {}", msg.received.data[0].data | fmt::FMT_HEX);
                break;
            }
        }
    }
}