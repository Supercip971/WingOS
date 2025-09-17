
#include <string.h>
#include "dev/pci/pci.hpp"
#include "iol/wingos/asset.hpp"
#include "iol/wingos/space.hpp"
#include "iol/wingos/syscalls.h"
#include "libcore/fmt/flags.hpp"
#include "libcore/fmt/log.hpp"
#include "libcore/type/trait.hpp"
#include "math/align.hpp"
#include "mcx/mcx.hpp"
#include "wingos-headers/asset.h"
#include "wingos-headers/syscalls.h"

struct [[gnu::packed]] SubmissionQueueEntry 
{
    uint8_t opcode;
    uint8_t fused_operation : 2;
    uint8_t _reserved : 4;
    uint8_t prp_select : 2;
    uint16_t command_identifier;

    uint32_t namspace_identifier;
    uint64_t _reserved2;
    uint64_t metadata;
    uint64_t data_pointer[2];
    uint32_t command_specific[6];
};

struct [[gnu::packed]] CompletionQueueEntry 
{
    uint32_t cmd_specific;
    uint32_t _reserved;
    uint32_t submition_queue_head_ptr;
    uint32_t submition_queue_identifier;
    uint32_t cmd_identifier;
    bool phase_bit : 1;
    uint16_t status : 15;
};

union [[gnu::packed]] ControllerCap
{
    struct [[gnu::packed]]{
        uint16_t max_queue_entries; 
        uint8_t contiguous_queue_req : 1;
        uint8_t arbitration_mechanism : 2;
        uint8_t _reserved : 5;
        uint8_t timeout; 
        uint8_t stride : 4;
        uint8_t reset_support : 1;
        uint8_t command_set_supported;
        uint8_t boot_partition_support : 1;
        uint8_t controller_power_scope : 2;
        uint8_t memory_page_size_minimum : 4;
        uint8_t memory_page_size_maximum : 4;
        uint8_t persistent_memory_region_supported : 1;
        uint8_t controller_memory_buffer_supported : 1;
        uint8_t nvm_subsystem_shutdown_supported : 1;
        uint8_t controller_ready_mode_supported :2;
    };
    
    struct {
        uint32_t raw_value_0;
        uint32_t raw_value_1;
    };
};



void dump_controller_cap(ControllerCap const& cap)
{
    log::log$("Controller Capability:");
    log::log$("- max queue entries           : {}", cap.max_queue_entries);
    log::log$("- contiguous queue requirement: {}", cap.contiguous_queue_req);
    log::log$("- timeout                     : {}", cap.timeout);
    log::log$("- stride                      : {}", cap.stride);
    log::log$("- mem page size minimum       : {}", cap.memory_page_size_minimum);
    log::log$("- mem page size maximum       : {}", cap.memory_page_size_maximum);
}
class NvmeDriver
{


    ControllerCap cap;
    uint64_t stride;
    template<typename T>
    struct Queue 
    {
        uint64_t base_addr;
        uint64_t count;
        bool is_completion;
        uint64_t tail;

        core::Result<T*> allocate()
        {
            T* res = (T*)(base_addr + tail*sizeof(T));

            tail+= 1; 

            if(tail >= count)
            {
                tail = 0;
                return allocate(); // circular  buffer
            }

            return res;
        }

        core::Result<Queue<T>> create(size_t count)
        {
            Queue<T> queue = {}; 
            
            size_t len = math::alignUp(count*sizeof(T), 4096ul);
            

            queue.count = math::alignDown(len, sizeof(T))/sizeof(T); 
            queue.is_completion = core::IsSame<T, CompletionQueueEntry>;
            queue.tail = 0;
            
            
            
            auto memory = Wingos::Space::self().allocate_physical_memory(len, false);
            if(memory.handle == 0)
            {
                return core::Result<Queue<T>>::error("failed to allocate memory for queue");
            }
            auto mapped = Wingos::Space::self().map_memory(memory, ASSET_MAPPING_FLAG_WRITE | ASSET_MAPPING_FLAG_EXECUTE);
            queue.base_addr = (uint64_t)mapped.ptr();
            memset((void*)queue.base_addr, 0, len);


            return queue;
        };

        core::Result<void> release()
        {
            Wingos::Space::self().release_memory(this->base_addr);
            return {};
        }



    };


    Queue<SubmissionQueueEntry> command_queue;
    Queue<CompletionQueueEntry> complete_queue;

    enum NvmeBaseReg
    {
        NVME_CAP = 0x0,
        NVME_VERSION = 0x8,
        NVME_INTERRUPT_MASK_SET = 0xc,
        NVME_INTERRUPT_MASK_CLEAR = 0x10,
        NVME_CONTROLLER_CONFIG = 0x14,
        NVME_CONTROLLER_STATUS = 0x1C,

        NVME_ADMIN_QUEUE_ATTRIBUTE = 0x24,
        NVME_ADMIN_QUEUE_SUBMIT = 0x28,
        NVME_ADMIN_QUEUE_COMPLETE = 0x30,

        NVME_QUEUE_TAIL_DOORBELL_BASE = 0x1000,
        NVME_QUEUE_HEAD_DOORBELL_BASE = 0x1000,
    };


    // Nvme express command set specification 1.2 - 3.3 NVM Command set Commands 
    enum NvmeOpcodes 
    {
        NVME_OP_FLUSH = 0x0,
        NVME_OP_WRITE = 0x1,
        NVME_OP_READ = 0x2,
        NVME_OP_WRITE_UNCORRECTABLE = 0x4,
        NVME_OP_COMPARE = 0x5, 
        NVME_OP_WRITE_ZERO = 0x8,
        NVME_OP_CANCEL = 0x18,
        NVME_OP_COPY = 0x19, 
        // ... other op in the doc but not really usefull 
    };

    Wingos::VirtualMemoryAsset mapped_nvme_addr_space;

    void *base_addr;
    void *remap_nvme_addr(uint64_t base, uint64_t len)
    {
        base = math::alignDown(base, 4096ul);
        len = math::alignUp(len + base, 4096ul) - base;

        mapped_nvme_addr_space = Wingos::Space::self().map_physical_memory(base, len, ASSET_MAPPING_FLAG_WRITE | ASSET_MAPPING_FLAG_EXECUTE);

        return mapped_nvme_addr_space.ptr();
    }
    uint32_t read(int reg)
    {
        void* base = mapped_nvme_addr_space.ptr();

        uint32_t volatile* offseted = reinterpret_cast<uint32_t volatile*>((uintptr_t)base + reg);
        
        return *offseted;
    }

    void write(int reg, uint32_t val)
    {
        void* base = mapped_nvme_addr_space.ptr();

        uint32_t volatile* offseted = reinterpret_cast<uint32_t volatile*>((uintptr_t)base + reg);
        
        *offseted = val;
    }

    public:

    static core::Result<NvmeDriver> setup(Wingos::dev::PciDevice &dev)
    {
        NvmeDriver driver;
        uint64_t device_addr = dev.get_bar64(Wingos::dev::pci_reg_dev_BAR0); // Get the BAR0 address

        auto len = dev.get_bar64_size(Wingos::dev::pci_reg_dev_BAR0);
        log::log$("Setting up NVMe disk at address: {} - {}", device_addr | fmt::FMT_HEX, (device_addr + len - 1) | fmt::FMT_HEX);

        driver.base_addr = driver.remap_nvme_addr(device_addr, len);

        driver.stride = (device_addr >> 12) & 0xf;

        log::log$("remapped nvme at: {}", ((uint64_t)driver.base_addr) | fmt::FMT_HEX);

        driver.cap.raw_value_0 = driver.read(NVME_CAP);
        driver.cap.raw_value_1 = driver.read(NVME_CAP + sizeof(uint32_t));

        dump_controller_cap(driver.cap);

        driver.command_queue = try$(Queue<SubmissionQueueEntry>().create(64));
        driver.complete_queue = try$(Queue<CompletionQueueEntry>().create(64));

        log::log$("Created command queue at: {}", driver.command_queue.base_addr | fmt::FMT_HEX);
        log::log$("Created completion queue at: {}", driver.complete_queue.base_addr | fmt::FMT_HEX);
        return driver;

    }

    core::Result<void> execute(uint8_t opcode,uint32_t nsid, uint64_t lba, uint32_t sector_count, void* data)
    {
        [[maybe_unused]] CompletionQueueEntry* completion = try$(complete_queue.allocate()); 
    
        SubmissionQueueEntry* submission = try$(command_queue.allocate());

        *submission = (SubmissionQueueEntry){};
        submission->opcode = opcode;
        submission->namspace_identifier =  nsid; 
        submission->command_specific[0] = lba;
        submission->command_specific[1] = (uint32_t)(lba >> 32);
        submission->command_specific[2] = (uint16_t)(sector_count-1);


        submission->data_pointer[0] = reinterpret_cast<uintptr_t>(data);
        submission->data_pointer[1] = 0;


        write(NVME_QUEUE_TAIL_DOORBELL_BASE + 2 * (4 << cap.stride), command_queue.tail);

        
        
        

        
        return {};

    }
};

int _main(mcx::MachineContext *)
{
    log::log$("hello world from nvme!");
    Wingos::dev::PciController pci_controller;
    pci_controller.scan_bus(0);

    core::Vec<NvmeDriver> disks;
    for (auto &dev : pci_controller.devices)
    {
        if (dev.class_code() == 0x01 && dev.subclass() == 0x08) // storage controller, NVMe
        {
            log::log$("Found NVMe device: Bus {}, Device {}, Function {}, Vendor ID: {}, Device ID: {}",
                      dev.bus, dev.device, dev.function,
                      dev.vendor_id() | fmt::FMT_HEX, dev.device_id() | fmt::FMT_HEX);
            auto disk = NvmeDriver::setup(dev);
            if (!disk.is_error())
            {
                disks.push(disk.unwrap());
            }
            else
            {
                log::err$("Failed to setup NVMe driver: {}", disk.error());
            }
        }
    }

    while (true)
    {

        asm volatile("pause");
    }
    return 1;
}