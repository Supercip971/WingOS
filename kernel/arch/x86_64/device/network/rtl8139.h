#ifndef RTL8139_H
#define RTL8139_H

#include <device/pci.h>
#include <stdint.h>

#define RTL_RECEIVE_BUFFER_SIZE (8192 + 16)

enum rtl_register
{
    RMAC = 0,
    RMAR = 8,
    RX_BUFFER_START = 0x30,
    COMMAND = 0x37,
    CURRENT_READ = 0x38,
    CURRENT_BUFFER_ADDR = 0x3a,
    INTERRUPT_MASK = 0x3C,
    INTERRUPT_SERVICE = 0x3E,
    TRANSMISSION_CONFIG_REG = 0x40,
    RECEIVE_CONFIG_REG = 0x44,
    CONFIG_1 = 0x52,

};

enum rtl_r_command
{
    RESTART = 0x10,
};

class rtl8139
{
    uint64_t io_addr;
    uint64_t mm_addr;

    uint8_t mac_address[6];

    void *rx_buffer;
    uint64_t rx_length;

    uint32_t read(uint32_t at);
    void write(uint32_t at, uint32_t data);

public:
    rtl8139();

    void interrupt_handle();
    void init(pci_device *device);

    static rtl8139 *the();
};

#endif // RTL8139_H
