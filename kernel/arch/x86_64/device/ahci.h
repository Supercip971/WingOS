#ifndef AHCI_H
#define AHCI_H
#include <device/pci.h>
#include <io_device.h>
enum fis_type_def
{
    FTYPE_REG_HOST2DEVICE = 0x27,
    FTYPE_REG_DEVICE2HOST = 0x34,
    FTYPE_DMA_ACTIVATE = 0x39,
    FTYPE_DMA_SETUP = 0x41,
    FTYPE_DATA = 0x46,
    FTYPE_BIST = 0x58,
    FTYPE_PIO_SETUP = 0x5F,
    FTYPE_DEVICE_BITS = 0xA1,
};
enum ahci_port_type
{
    NULL_PORT = 0,
    SATA_PORT = 1,
    SEMB_PORT = 2,
    PM_PORT = 3,
    SATAPI_PORT = 3,
};
enum sata_sig_type
{
    SATA_SIG_ATA = 0x101,
    SATA_SIG_ATAPI = 0xEB140101,
    SATA_SIG_SEMB = 0xC33C0101,
    SATA_SIG_PM = 0x96690101,
};

enum hba_port_state
{
    HBA_PORT_IPM_ACTIVE = 1,
    HBA_PORT_DET_PRESENT = 3,
};

enum hba_port_command
{
    COMMAND_ST = 0x1,
    COMMAND_FRE = 0x10,
    COMMAND_FR = 0x4000,
    COMMAND_CR = 0x8000,
};
#define DMA_ATA_READ_EXTENDED 0x25
#define TASK_FILE_ERROR (1 << 30)
enum ata_device_status
{
    ATA_DEV_BUSY = 0x80,
    ATA_DEV_DRQ = 0x08,
};

struct fis_reg_host2device
{
    uint8_t type;
    uint8_t port_multiplier : 4;
    uint8_t reserved : 3;
    uint8_t cmd_or_ctrl : 1; // command=1 control=0
    uint8_t command_register;
    uint8_t feature_low;
    uint8_t lba0;
    uint8_t lba1;
    uint8_t lba2;
    uint8_t device;
    uint8_t lba3;
    uint8_t lba4;
    uint8_t lba5;
    uint8_t feature_high;
    uint8_t count_low;
    uint8_t count_high;
    uint8_t isochronous_command_completion;
    uint8_t control;
    uint8_t reservec[4];
} __attribute__((packed));

struct fis_reg_device2host
{
    uint8_t type;
    uint8_t port_multiplier : 4;
    uint8_t reserved : 2;
    uint8_t interrupt : 1; // command=1 control=0
    uint8_t reserved_1 : 1;
    uint8_t status;
    uint8_t error;

    uint8_t lba0;
    uint8_t lba1;
    uint8_t lba2;
    uint8_t device;

    uint8_t lba3;
    uint8_t lba4;
    uint8_t lba5;
    uint8_t reserved_2;
    uint8_t count_low;
    uint8_t count_high;
    uint8_t reserved_3[5];
} __attribute__((packed));

struct fis_reg_data
{
    uint8_t type;
    uint8_t port_multiplier : 4;
    uint8_t reserved : 4;
    uint8_t reserved_1[2];
    uint32_t data[1];

} __attribute__((packed));

struct fis_pio_setup
{
    uint8_t type;
    uint8_t port_multiplier : 4;
    uint8_t reserved : 1;
    uint8_t data_transfer_direction : 1; // 1 = device -> host
    uint8_t interrupt : 1;
    uint8_t reserved_1 : 1;
    uint8_t status;
    uint8_t error;

    uint8_t lba0;
    uint8_t lba1;
    uint8_t lba2;
    uint8_t device;

    uint8_t lba3;
    uint8_t lba4;
    uint8_t lba5;
    uint8_t reserved_2;

    uint8_t count_low;
    uint8_t count_high;
    uint8_t reserved_3;
    uint8_t new_status;
    uint16_t transfer_count;
    uint8_t reserved_4[2];
} __attribute__((packed));

struct fis_dma_setup
{
    uint8_t type;
    uint8_t port_multiplier : 4;
    uint8_t reserved : 1;
    uint8_t data_transfer_direction : 1; // 1 = device -> host
    uint8_t interrupt : 1;
    uint8_t auto_acrivate : 1; // if dma is on then FIS is needed
    uint8_t reserved_1[2];
    uint64_t dma_buffer_identifier;
    uint32_t reserved_2;
    uint32_t dma_buffer_offset;
    uint32_t transfer_count;
    uint32_t reserved_3;
} __attribute__((packed));

struct hba_port
{
    uint32_t command_list_base_addr_low;
    uint32_t command_list_base_addr_up;
    uint32_t fis_base_addr_low;
    uint32_t fis_base_addr_up;
    uint32_t interrupt_status;
    uint32_t interrupt_enable;
    uint32_t command_and_status;
    uint32_t reserved;
    uint32_t task_file_data;
    uint32_t signature;
    uint32_t sata_status;
    uint32_t sata_control;
    uint32_t sata_error;
    uint32_t sata_active;
    uint32_t command_issue;
    uint32_t sata_notification;
    uint32_t fis_based_switch_control;
    uint32_t reserved_1[11];
    uint32_t vendor[4];
} __attribute__((packed));

struct hba_memory
{
    uint32_t capability;
    uint32_t global_host_control;
    uint32_t interrupt_status;
    uint32_t port_implemented;
    uint32_t version;

    uint32_t command_completion_coalescin_control;
    uint32_t command_completion_coalescin_ports;

    uint32_t enclosuze_management_location;
    uint32_t enclosuze_management_control;

    uint32_t capability_extended;
    uint32_t bios_handoff_control_status;

    uint8_t reserved[116];
    uint8_t vendor[96];

    hba_port ports[1];
} __attribute__((packed));

struct hba_fis
{
    fis_dma_setup fsfis;
    uint8_t padding_0[4];

    fis_pio_setup psfis;
    uint8_t padding_1[12];

    fis_reg_device2host rfis;
    uint8_t padding_2[4];

    uint8_t un_iloveosdev1;
    uint8_t un_iloveosdev2;
    uint8_t padding_3[64]; // padding EVERYWHERE
    uint8_t reserved[96];
} __attribute__((packed));

struct hba_cmd_header
{
    uint8_t command_fis_length : 5;

    uint8_t atapi : 1;
    uint8_t write : 1; // 1 = host -> device | 0 = device -> host
    uint8_t prefetchable : 1;
    uint8_t reset : 1;
    uint8_t bist : 1;
    uint8_t clear_busy_upon_ok : 1;
    uint8_t reserved_0 : 1;

    uint8_t port_multiplier_port : 3;

    uint16_t physical_region_descriptor_table_entry_count;
    uint32_t physical_region_descriptor_byte_count;

    uint32_t command_table_desc_base_addr;
    uint32_t command_table_desc_base_addr_up;

    uint32_t reserved[4];

} __attribute__((packed));

struct hba_prdt_entry
{
    uint32_t data_base_addr_low;
    uint32_t data_base_addr_up;
    uint32_t reserved;

    uint32_t dbc : 22;
    uint32_t reserved_1 : 9;
    uint32_t interrupt_on_complete : 1;
} __attribute__((packed));

struct hba_command_table
{
    uint8_t command_fis[64];
    uint8_t atapi_command[16];
    uint8_t reserved[48];
    hba_prdt_entry entry[1]; // yeah buffer overflow bla bla bla
}
__attribute__((packed));

class ahci : public pci_device_driver
{
    hba_memory *hba_mem;
    pci_bar_data ahci_bar;
    uint64_t data_addr;
    void start_command(hba_port *port);
    void end_command(hba_port *port);
    ahci_port_type check_device_type(hba_port *port) const;
    void reinit_port(hba_port *port);

public:
    ahci(pci_device v) : pci_device_driver(v){};
    void init();
};

class ahci_ata_device : public generic_io_device
{
    volatile hba_port *port;
    int find_command_slot();

    void start_command();
    void end_command();

public:
    ahci_ata_device()
    {
    }
    void set_port(hba_port *new_port)
    {
        port = new_port;
    }
    void init();
    io_rw_output read(uint8_t *data, uint64_t count, uint64_t cursor) override;
    io_rw_output write(uint8_t *data, uint64_t count, uint64_t cursor) override;
    const char *get_name() const final
    {
        return "ahci ata device";
    }
};

#endif // AHCI_H
