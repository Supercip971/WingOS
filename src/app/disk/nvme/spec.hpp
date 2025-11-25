#pragma once 
#include <stdint.h>
#include <libcore/fmt/log.hpp>

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

static inline void dump_controller_cap(ControllerCap const &cap)
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
