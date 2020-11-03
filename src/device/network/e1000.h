#pragma once
#include <arch/64bit.h>
#include <device/pci.h>
#include <stdint.h>

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
    ENABLE = 1,
    STORE_BAD_PACKET,
    UNICAST_ENABLE,
    MULTICAST_ENABLE,
    LONG_PACKET_ENABLE,
    NO_LOOPBACK = 6,
    PHY_LOOPBACK = 6,
    THRESHOLD = 8,
    MULTICAST_OFFSET = 12,
    BROADCAST_ACCEPT = 15,
    VLAN_FILTER = 18,
    CANONICAL_ENABLE,
    CANONICAL_VALUE,
    DISCARD_PAUSE_FRAME,
    MAC_CONTROL_FRAME,
    STRIP_ETHERNET = 26
};
#define FOFF(a) (1 << a)
#define RCONTROL_SIZE_256 (3 << 16)
#define RCONTROL_SIZE_512 (2 << 16)
#define RCONTROL_SIZE_1024 (1 << 16)
#define RCONTROL_SIZE_2048 (0 << 16)
#define RCONTROL_SIZE_4096 ((3 << 16) | (1 << 25))
#define RCONTROL_SIZE_8192 ((2 << 16) | (1 << 25))
#define RCONTROL_SIZE_16384 ((1 << 16) | (1 << 25))

enum e1000_TRANSMIT_COMMAND
{
    END_OF_PACKET = 0,
    INSERT_FCS,
    INSERT_CHECK_SUM,
    REPORT_STATUS,
    REPORT_PACKET_SEND,
    VLAN_ENABLE,
    INTERRUPT_DELAY_ENABLE
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
struct mac_address
{
    uint8_t mac[6];
};
class e1000
{
    uint8_t bar_t;
    uint16_t io_base_addr;
    uint64_t mm_address;
    bool does_eerprom_exists;
    mac_address maddr;
    struct rx_desc
    {
        uint64_t address;
        uint16_t length;
        uint16_t checksum;
        uint8_t status;
        uint8_t errors;
        uint16_t special;
    } __attribute__((packed));
    rx_desc *rx_descriptor[RX_DESCRIPTOR_COUNT];
    uint16_t rx_current_buf;
    struct tx_desc
    {
        uint64_t address;
        uint16_t length;
        uint8_t cso;
        uint8_t command;
        uint8_t status;
        uint8_t css;
        uint16_t special;
    } __attribute__((packed));

    tx_desc *tx_descriptor[TX_DESCRIPTOR_COUNT];
    uint16_t tx_current_buf;

    void write(uint16_t addr, uint32_t val);
    uint32_t read(uint16_t addr);
    uint32_t errp_rom_read(uint8_t addr);

    bool eerp_rom_detection();
    bool mac_detection();
    void start();
    void setup_tx();
    void setup_rx();
    void turn_on_int();
    void handle_packet_reception();

public:
    e1000();
    void init(pci_device *dev);
    static e1000 *the();
    void irq_handle(InterruptStackFrame *frame);
    constexpr mac_address get_mac_addr()
    {
        return maddr;
    }
    int send_packet(uint8_t *data, uint16_t length);
    void update();
};
