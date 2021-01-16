#include <arch.h>
#include <device/network/rtl8139.h>
#include <liballoc.h>
#include <logging.h>
#include <virtual.h>
rtl8139 main_rtl;

uint32_t rtl8139::read(uint32_t at)
{

    return inl(mm_addr + at);
}
void rtl8139::write(uint32_t at, uint32_t data)
{
    outl(mm_addr + at, data);
}
rtl8139::rtl8139()
{
}

rtl8139 *rtl8139::the()
{
    return &main_rtl;
}

void rtl8139::init(pci_device *device)
{
    mm_addr = device->get_bar(0).base;
    update_paging();
    log("rtl8139", LOG_DEBUG) << "loadingt rtl8139 addr : " << mm_addr;
    write(CONFIG_1, 0x0);

    write(COMMAND, 0x10);
    while ((read(COMMAND) & 0x10) != 0)
    {
    };
    rx_buffer = malloc(RTL_RECEIVE_BUFFER_SIZE);
    for (int i = 0; i < RTL_RECEIVE_BUFFER_SIZE; i++)
    {
        ((char *)rx_buffer)[i] = 0;
    }

    write(INTERRUPT_MASK, 0x0005);
    write(RECEIVE_CONFIG_REG, 0xf | (1 << 7));
    write(COMMAND, 0xc);
    write(RECEIVE_CONFIG_REG, 0xf | (1 << 7)); // i don't know, why not ?
    uint32_t low_mac_addr = read(RMAC);
    uint32_t high_mac_addr = read(RMAC + 0x4);
    mac_address[0] = (uint8_t)(low_mac_addr >> 0);
    mac_address[1] = (uint8_t)(low_mac_addr >> 8);
    mac_address[2] = (uint8_t)(low_mac_addr >> 16);
    mac_address[3] = (uint8_t)(low_mac_addr >> 24);

    mac_address[4] = (uint8_t)(high_mac_addr >> 0);
    mac_address[5] = (uint8_t)(high_mac_addr >> 8);
    for (int i = 0; i < 6; i++)
    {
        log("rtl8139", LOG_INFO) << "mac " << i << " = " << mac_address[i];
    }
}
