
#include <string.h>
#include <sys/types.h>

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

uint64_t device_uid;
struct [[gnu::packed]] CompletionQueueEntry
{
    uint32_t cmd_specific;
    uint32_t _reserved;
    uint16_t submission_queue_head_ptr;
    uint16_t submission_queue_identifier;
    uint16_t cmd_identifier;
    bool phase_bit : 1;
    uint16_t status : 15;
};

union [[gnu::packed]] ControllerCap
{
    struct [[gnu::packed]]
    {
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
        uint8_t controller_ready_mode_supported : 2;
    };

    struct
    {
        uint32_t raw_value_0;
        uint32_t raw_value_1;
    };
};

union [[gnu::packed]] NvmeConfig
{
    struct [[gnu::packed]]
    {
        // name used in figure 78 (3.1.5) of the NVMe spec
        uint8_t en : 1; // enable
        uint8_t _reserved : 3;
        uint8_t css : 3;  // IO Command Set Selected
        uint8_t mps : 4;  // memory page size
        uint16_t ams : 3; // arbitration mechanism selected

        uint8_t shn : 2;    // shutdown notification
        uint8_t iosqes : 4; // I/O Submission Queue Entry Size
        uint8_t iocqes : 4; // I/O Completion Queue Entry Size
    };
    uint32_t raw_value;
};

void dump_controller_cap(ControllerCap const &cap)
{
    log::log$("Controller Capability:");
    log::log$("- max queue entries           : {}", cap.max_queue_entries);
    log::log$("- contiguous queue requirement: {}", cap.contiguous_queue_req);
    log::log$("- timeout                     : {}", cap.timeout);
    log::log$("- stride                      : {}", cap.stride);
    log::log$("- mem page size minimum       : {}", cap.memory_page_size_minimum);
    log::log$("- mem page size maximum       : {}", cap.memory_page_size_maximum);
}

struct [[gnu::packed]] NvmeCmdHeader
{
    uint8_t opcode;
    uint8_t fuse : 2;
    uint8_t _reserved : 4;
    uint8_t psdt : 2;
    uint16_t cid;
};

struct [[gnu::packed]] NvmeCmd
{
    NvmeCmdHeader header;

    uint32_t nsid;
    uint64_t reserved;
    uint64_t mptr;
    uint64_t prp1;
    uint64_t prp2;
    union
    {
        struct [[gnu::packed]]
        {
            uint32_t cdw10;
            uint32_t cdw11;
            uint32_t cdw12;
            uint32_t cdw13;
            uint32_t cdw14;
            uint32_t cdw15;
        } raw;

        struct [[gnu::packed]]
        {
            uint16_t qid;   // queue id
            uint16_t qsize; // queue size
            bool pc : 1;    // physically contiguous
            bool ien : 1;   // interrupt enable
            uint32_t _reserved : 14;
            uint16_t iv; // interrupt vector
        } CreateIoCompletionQueue;
        // Results:
        // 0x1 : Invalid queue identifier
        // 0x2 : Invalid queue size
        // 0x8 : Invalid interrupt vector
        struct [[gnu::packed]]
        {
            uint16_t qid;      // queue id
            uint16_t qsize;    // queue size
            bool pc : 1;       // physically contiguous
            uint8_t qprio : 2; // queue priority
            uint32_t _reserved : 13;
            uint16_t cqid; // Completion queue identifier

            uint16_t nvmsetid; // nvme set id
            uint16_t _reserved2;
        } CreateIoSubmissionQueue;
        // results:
        // 0 : Invalid completion queue

        struct [[gnu::packed]]
        {
            uint8_t cns; // namspace structure or controller
            uint8_t _reserved;
            uint16_t cntid;    // controller identifier
            uint16_t nvmsetid; // NVMe set identifier
            uint16_t _reserved2;
            uint8_t uuid_idx : 7;
            uint32_t _reserved3 : 25;
        } Identify;

        struct [[gnu::packed]]
        {
            uint64_t lba;
            uint16_t nlb; // number of logical block
            uint16_t _reserved : 4;
            uint8_t dtype : 4; // directive type (0 for read)
            uint8_t _reserved2 : 2;
            uint8_t prinfo : 4; // protection information
            uint8_t fua : 1;    // force unit access
            uint8_t lr : 1;     // limited retry
        } ReadWrite;

        // Results:
        // 0x20 = namespace is write protected
        // 0x80 = Conflicting attributes
        // 0x81 = Invalid protection information
        // 0x82 = Writing a read only

        struct [[gnu::packed]]
        {
            uint32_t fid; // feature identifier + select
            uint32_t dword11;

        } Feature;
    };
};

// static auto v=  offsetof(NvmeCmd, raw.cdw11);

struct [[gnu::packed]] NvmeLBAf
{
    uint16_t ms;    // metadata size
    uint8_t lbads;  // lba data size
    uint8_t rp : 2; // relative performance
    uint8_t _reserved : 6;
};

union [[gnu::packed]] NvmeIdentifyNamespace
{
    struct
    {
        uint64_t nsze;        // namespace size in logical blocks
        uint64_t ncap;        // namespace capacity in logical blocks
        uint64_t nuse;        // namespace utilization in logical blocks
        uint8_t thinp : 1;    // thin provisioning
        uint8_t nsabp : 1;    // namespace atomic write unit
        uint8_t dae : 1;      // supports deallocated or unwritten logical blocks error support
        uint8_t uidreuse : 1; // unique identifier reuse
        uint8_t optperf : 1;  // usage of NPWG, NPWA, NPDG, NPDA, and NOWS or just NOWS
        uint8_t _reserved : 3;
        uint8_t nlbaf; // number of lba formats
        uint8_t flbas; // formatted lba size
        uint8_t mc;    // metadata capabilities
        uint8_t dpc;
        uint8_t dps;        // data protection capabilities
        uint8_t nmic;       // namespace multi-path I/O and namespace sharing capabilities
        uint8_t rescap;     // reservation capabilities
        uint8_t fpi;        // format progress indicator
        uint8_t dlfeat;     // deallocate logical block features
        uint16_t nawun;     // namespace atomic write unit normal
        uint16_t nawupf;    // namespace atomic write unit power fail
        uint16_t nacwu;     // namespace atomic compare and write unit
        uint16_t nabsn;     // namespace atomic boundary size normal
        uint16_t nabo;      // namespace atomic boundary offset
        uint16_t nabspf;    // namespace atomic boundary size power fail
        uint16_t noiob;     // optimal I/O boundary
        uint64_t nvmcap[2]; // NVM capacity
        uint16_t npwg;      // namespace preferred write granularity
        uint16_t npwa;      // namespace preferred write alignment
        uint16_t npdg;      // namespace preferred deallocate granularity
        uint16_t npda;      // namespace preferred deallocate alignment
        uint16_t nows;      // namespace optimal write size
        uint8_t _reserved1[18];
        uint32_t anagrpid; // ANA group identifier
        uint8_t _reserved2[3];
        uint8_t nsattr;    // namespace attributes
        uint16_t nvmsetid; // NVM set identifier
        uint16_t endgid;   // endurance group identifier

        uint64_t nguid[2];             // namespace globally unique identifier
        uint64_t eui64;                // IEEE extended unique identifier
        NvmeLBAf lbaf[16];             // lba format
        uint8_t _reserved3[383 - 191]; // vendor specific
    };

    uint8_t raw[4096];
};

union [[gnu::packed]] NvmeIdentifyController
{
    struct [[gnu::packed]]
    {
        uint16_t vid;     // PCI vendor id
        uint16_t ssvid;   // PCI subsystem vendor id
        char sn[23 - 3];  // serial number
        char mn[63 - 23]; // model number
        char fr[71 - 63]; // firmware revision
        uint8_t rab;      // recommended arbitration burst
        uint8_t ieee[3];  // ieee oui identifier
        uint8_t cmic;     // controller multi-path I/O and namespace sharing capabilities
        uint8_t mdts;     // maximum data transfer size
        uint16_t cntlid;  // controller identifier
        uint32_t ver;     // version
        uint32_t rtd3r;   // RTD3 resume latency
        uint32_t rtd3e;   // RTD3 entry latency
        uint32_t oaes;    // optional asynchronous events supported
        uint32_t ctratt;  // controller attributes
        uint16_t rrls;    // read recovery levels supported

        uint8_t _reserved[110 - 101];
        uint8_t cntrltype;
        uint64_t fguid[2]; // FRU Globally Unique Identifier
        uint16_t crdt1;    // Command Retry Delay Time 1
        uint16_t crdt2;    // Command Retry Delay Time 2
        uint16_t crdt3;    // Command Retry Delay Time 3

        uint8_t _reserved2[255 - 133];
        uint16_t oacs;       // optional admin command support
        uint8_t acl;         // abort command limit
        uint8_t aerl;        // asynchronous event request limit
        uint8_t frmw;        // firmware updates
        uint8_t lpa;         // log page attributes
        uint8_t elpe;        // error log page entries
        uint8_t npss;        // number of power states support
        uint8_t avscc;       // admin vendor specific command configuration
        uint8_t apsta;       // autonomous power state transition attributes
        uint16_t wctemp;     // warning composite temperature threshold
        uint16_t cctemp;     // critical composite temperature threshold
        uint16_t mtfa;       // maximum time for firmware activation
        uint32_t hmpre;      // host memory buffer preferred size
        uint32_t hmmin;      // host memory buffer minimum size
        uint64_t tnvmcap[2]; // total NVM capacity
        uint64_t unvmcap[2]; // unallocated NVM capacity
        uint32_t rpmbs;      // replay protected memory block support
        uint16_t edstt;      // extended device self-test time
        uint8_t dsto;        // device self-test options
        uint8_t fwug;        // firmware update granularity
        uint16_t kas;        // keep alive support
        uint16_t hctma;      // host controlled thermal management attributes
        uint16_t mntmt;      // minimum thermal management temperature
        uint16_t mxntmt;     // maximum thermal management temperature
        uint32_t sanicap;
        uint32_t hmminds;   // host memory buffer minimum descriptor entry size
        uint16_t hmmaxd;    // host memory buffer maximum descriptor entry size
        uint16_t nsetidmax; // NVM set identifier maximum
        uint16_t endgidmax; // endurance group identifier maximum
        uint8_t anatt;      // ANA transition time
        uint8_t anacap;     // ANA capabilities
        uint32_t anagrpmax; // ANA group identifier maximum
        uint32_t nanagrpid; // number of ANA group identifiers
        uint32_t pels;      // persistent event log size
        uint8_t _reserved3[511 - 355];

        uint8_t sqes;    // submission queue entry size
        uint8_t cqes;    // completion queue entry size
        uint16_t maxcmd; // maximum outstanding commands
        uint32_t nn;     // number of namespaces
        uint16_t oncs;   // optional NVM command support
        uint16_t fuses;  // fused operation support
        uint8_t fna;     // format NVM attributes
        uint8_t vwc;     // volatile write cache
        uint16_t awun;   // atomic write unit normal
        uint16_t awupf;  // atomic write unit power fail
        uint8_t nvscc;   // NVM vendor specific command configuration
        uint8_t nwpc;    // namespace write protection capabilities
        uint16_t acwu;   // atomic compare and write unit
        uint16_t _reserved4;
        uint32_t sgls; // SGL support
        uint32_t mnan; // maximum number of namespaces
        uint8_t _reserved5[767 - 543];
        char subnqn[1023 - 767]; // NVM Subsystem NVMe Qualified Name (0 terminated)
        uint8_t _reserved6[4096 - 1024];
    };

    uint8_t raw[4096];
};
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
enum NvmeAdminOpcode
{
    NVME_AOP_DELETE_SUBMISSION_QUEUE = 0x0,
    NVME_AOP_CREATE_SUBMISSION_QUEUE = 0x1,
    NVME_AOP_GET_LOG = 0x2,
    NVME_AOP_DELETE_COMPLETION_QUEUE = 0x4,
    NVME_AOP_CREATE_COMPLETION_QUEUE = 0x5,
    NVME_AOP_IDENTIFY = 0x6,
    NVME_AOP_SET_FEATURE = 0x9,
    NVME_AOP_GET_FEATURE = 0xA,

    // ... other op in the doc but not really usefull
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
            Wingos::Space::self().release_memory(this->base_addr, this->page_size);
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

        log::log$(" - Namespace {}: {} blocks of size {}", nsid, device.identify_namespace->nsze, 1 << (device.identify_namespace->lbaf[device.identify_namespace->flbas & 0xf].lbads));

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
        log::log$("- Vendor ID       : {}", driver.identify_controller->vid | fmt::FMT_HEX);
        log::log$("- Subsystem VID   : {}", driver.identify_controller->ssvid | fmt::FMT_HEX);
        log::log$("- Serial Number   : {}", core::Str(driver.identify_controller->sn, 20));
        log::log$("- Model Number    : {}", core::Str(driver.identify_controller->mn, 40));
        log::log$("- Firmware Revision : {}", core::Str(driver.identify_controller->fr, 8));
        log::log$("- Max Data Transfer Size : {}", 1 << driver.identify_controller->mdts);
        log::log$("- Number of Namespaces : {}", driver.identify_controller->nn);
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

        log::log$("Found {} namespaces:", driver.identify_controller->nn);

        try$(driver.nvme_set_queue_count(4));
        for (uint32_t i = 0; i < driver.identify_controller->nn; i++)
        {
            if (driver.nsids[i] == 0)
                continue;

            if (driver.nsids[i] > driver.identify_controller->mnan && driver.identify_controller->mnan != 0)
            {
                log::err$("Namespace ID {} is greater than maximum number of namespaces {}", driver.nsids[i], driver.identify_controller->mnan);
                continue;
            }
            log::log$("- Namespace ID: {}", driver.nsids[i]);
            try$(driver.nvme_register_device_namespace(driver.nsids[i]));
        }

        return driver;
    }
};

int _main(mcx::MachineContext *)
{

    // convert this in gnu format: asm volatile("and rsp, -16");

    log::log$("hello world from nvme!");
    Wingos::dev::PciController pci_controller;
    pci_controller.scan_bus(0);

    device_uid = 0;

    core::Vec<NvmeController> disks;
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
                disks.push(disk.unwrap());

                auto mapped = Wingos::Space::self().allocate_memory(4096, false);
                log::log$("Allocated memory at: {}", (uintptr_t)mapped.ptr() | fmt::FMT_HEX);

                disks[0].read_write_ptr(&disks[0].devices[0], false, 0, 8, mapped.ptr(), 4096);

                /*
                for (size_t i = 0; i < 512; i++)
                {
                    if (i % 16 == 0)
                    {
                        log::log$("\n{}:", i * 8);
                    }
                    log::log$(" {}{}", ((uint8_t *)mapped.ptr())[i * 2] | fmt::FMT_HEX, ((uint8_t *)mapped.ptr())[i * 2 + 1] | fmt::FMT_HEX);
                }*/

                log::log$("NVMe worked !");
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
}