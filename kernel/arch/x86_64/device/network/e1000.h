#pragma once
#include <64bit.h>
#include <device/network/ethernet_protocol.h>
#include <device/pci.h>
#include <stdint.h>
#include <utils/container/wvector.h>

#define E_INTEL_VENDOR 0x8086
#define E_DEVICE 0x100e
#define E_DEVICE_I217 0x153a
#define E_DEVICE_82577LM 0x10ea

enum e1000_registers
{
    E_CONTROL = 0,
    E_STATUS = 8,
    E_EEPROM = 0x14,
    E_CONTROL_EXT = 0x18,
    E_IMASK = 0xD0,
    E_INTERRUPT_RATE = 0xC4,
    E_RCONTROL = 0x100,
    E_RX_DESCRIPTOR_LOW = 0x2800,
    E_RX_DESCRIPTOR_HIGH = 0x2804,
    E_RX_DESCRIPTOR_LENGTH = 0x2808,
    E_RX_DESCRIPTOR_HEAD = 0x2810,
    E_RX_DESCRIPTOR_TAIL = 0x2818,

    E_RX_DELAY_TIMER = 0x2820,
    E_RX_DESCRIPTOR_CONTROL = 0x3828,
    E_RX_ABS_DELAY_TIMER = 0x282C,
    E_RX_SMALL_PACKET_DETECT = 0x2C00,

    E_TCONTROL = 0x400,
    E_TX_DESCRIPTOR_LOW = 0x3800,
    E_TX_DESCRIPTOR_HIGH = 0x3804,
    E_TX_DESCRIPTOR_LENGTH = 0x3808,
    E_TX_DESCRIPTOR_HEAD = 0x3810,
    E_TX_DESCRIPTOR_TAIL = 0x3818,

    E_TRANSMIT_INTER_PACKET_GAP = 0x410,
    E_CONTROL_SLU = 0x40,
};

enum e1000_RCONTROL
{
    ENABLE = (1 << 1),
    STORE_BAD_PACKET = (1 << 2),
    UNICAST_ENABLE = (1 << 3),
    MULTICAST_ENABLE = (1 << 4),
    LONG_PACKET_ENABLE = (1 << 5),
    NO_LOOPBACK = (0 << 6),
    PHY_LOOPBACK = (3 << 6),
    THRESHOLD = (0 << 8),
    MULTICAST_OFFSET = (0 << 12),
    BROADCAST_ACCEPT = (1 << 15),
    VLAN_FILTER = ((1 << 18)),
    CANONICAL_ENABLE = ((1 << 19)),
    CANONICAL_VALUE = ((1 << 20)),
    DISCARD_PAUSE_FRAME = (1 << 22),
    MAC_CONTROL_FRAME = (1 << 23),
    STRIP_ETHERNET = (1 << 26)
};

#define RCONTROL_SIZE_256 (3 << 16)
#define RCONTROL_SIZE_512 (2 << 16)
#define RCONTROL_SIZE_1024 (1 << 16)
#define RCONTROL_SIZE_2048 (0 << 16)
#define RCONTROL_SIZE_4096 ((3 << 16) | (1 << 25))
#define RCONTROL_SIZE_8192 ((2 << 16) | (1 << 25))
#define RCONTROL_SIZE_16384 ((1 << 16) | (1 << 25))

enum e1000_TRANSMIT_COMMAND
{
    END_OF_PACKET = (1 << 0),
    INSERT_FCS = (1 << 1),
    INSERT_CHECK_SUM = (1 << 2),
    REPORT_STATUS = (1 << 3),
    REPORT_PACKET_SEND = (1 << 4),
    VLAN_ENABLE = (1 << 6),
    INTERRUPT_DELAY_ENABLE = (1 << 7)
};

enum e1000_TCTL_REGISTERS
{
    TRANSMIT_ON = (1 << 1),
    PAD_SHORT_PACKET = (1 << 3),
    COLLISION_THRESHOLD = 4,
    COLLISION_DISTANCE = 12,
    SOFTWARE_XOFF = (1 << 22),
    RE_SEND_ON_LATE = (1 << 24),
    DESC_DONE = (1 << 0),
    EXCESS_COLLISION = (1 << 1),
    LATE_COLLISION = (1 << 2),
    TRANSMIT_UNDER_RUN = (1 << 3)
};

#define RX_DESCRIPTOR_COUNT 32
#define TX_DESCRIPTOR_COUNT 8

class e1000
{
    uint8_t bar_t;

    uint16_t io_base_addr;
    uint64_t mm_address;
    utils::lock_type packet_reception_lock;

    utils::vector<void *> packet_reception_list;
    bool eerp_rom_detection();
    bool does_eerprom_exists;

    bool mac_detection();
    mac_address maddr;

    struct rx_desc
    {
        volatile uint64_t address;
        volatile uint16_t length;
        volatile uint16_t checksum;
        volatile uint8_t status;
        volatile uint8_t errors;
        volatile uint16_t special;
    } __attribute__((packed));

    rx_desc *rx_descriptor[RX_DESCRIPTOR_COUNT];
    uint16_t rx_current_buf = 0;
    void setup_rx();

    struct tx_desc
    {
        volatile uint64_t address;
        volatile uint16_t length;
        volatile uint8_t cso;
        volatile uint8_t command;
        volatile uint8_t status;
        volatile uint8_t css;
        volatile uint16_t special;
    } __attribute__((packed));

    tx_desc *tx_descriptor[TX_DESCRIPTOR_COUNT];
    uint16_t tx_current_buf = 0;
    void setup_tx();

    void write(uint16_t addr, uint32_t val);
    uint32_t read(uint16_t addr);
    uint32_t errp_rom_read(uint8_t addr);

    void start();
    void turn_on_int();
    void handle_packet_reception();

public:
    e1000();
    void init(pci_device *dev);
    static e1000 *the();
    void irq_handle();

    void *read_last_packet();
    constexpr mac_address &get_mac_addr()
    {
        return maddr;
    }
    int send_packet(uint8_t *data, uint16_t length);
    void update();
};
