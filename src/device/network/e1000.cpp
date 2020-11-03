#include <arch/arch.h>
#include <arch/mem/liballoc.h>
#include <device/network/e1000.h>
#include <logging.h>
e1000 main_e1000;
e1000::e1000()
{
}

e1000 *e1000::the()
{
    return &main_e1000;
}

void e1000::update()
{
}

void e1000::write(uint16_t addr, uint32_t val)
{
    if (bar_t == 0)
    {
        *((volatile uint32_t *)(mm_address + addr)) = val;
    }
    else
    {
        outl(io_base_addr, addr);
        outl(io_base_addr + 4, val);
    }
}
uint32_t e1000::read(uint16_t addr)
{
    if (bar_t == 0)
    {
        return *((volatile uint32_t *)(mm_address + addr));
    }
    else
    {

        outl(io_base_addr, addr);
        return inl(io_base_addr + 4);
    }
}
bool e1000::eerp_rom_detection()
{
    uint32_t val = 0;
    write(E_EEPROM, 0x1);

    for (int i = 0; i < 1000 && !does_eerprom_exists; i++)
    {
        val = read(E_EEPROM);
        if (val & 0x10)
        {
            does_eerprom_exists = true;
        }
        else
        {
            does_eerprom_exists = false;
        }
    }
    return does_eerprom_exists;
}

uint32_t e1000::errp_rom_read(uint8_t addr)
{
    uint16_t tmp_data = 0;
    uint32_t tmp_2 = 0;
    if (does_eerprom_exists)
    {
        write(E_EEPROM, 1 | ((uint32_t)addr << 8));

        while (true)
        {
            tmp_2 = read(E_EEPROM) & (1 << 4);
            if (tmp_2)
            {
                break;
            }
        }
    }
    else
    {
        write(E_EEPROM, 1 | ((uint32_t)addr << 2));

        while (true)
        {
            tmp_2 = read(E_EEPROM) & (1 << 1);
            if (tmp_2)
            {
                break;
            }
        }
    }

    return ((uint16_t)tmp_2 >> 16) & 0xffff;
}

bool e1000::mac_detection()
{
    if (does_eerprom_exists)
    {

        uint32_t temp;
        temp = errp_rom_read(0);
        maddr.mac[0] = temp;
        maddr.mac[1] = temp >> 8;
        temp = errp_rom_read(1);
        maddr.mac[2] = temp;
        maddr.mac[3] = temp >> 8;
        temp = errp_rom_read(2);
        maddr.mac[4] = temp;
        maddr.mac[5] = temp >> 8;
    }
    else
    {
        uint8_t *base_8 = (uint8_t *)(mm_address + 0x5400);
        uint32_t *base_32 = (uint32_t *)(mm_address + 0x5400);
        if (base_32[0] != 0)
        {
            for (int i = 0; i < 6; i++)
            {
                maddr.mac[i] = base_8[i];
            }
        }
        else
        {
            return false;
        }
    }
    return true;
}

void e1000::setup_rx()
{
    uint8_t *tmp_ptr;
    rx_desc *descriptor;
    tmp_ptr = (uint8_t *)malloc(sizeof(rx_desc) * RX_DESCRIPTOR_COUNT + 16);
    descriptor = (rx_desc *)tmp_ptr;
    for (int i = 0; i < RX_DESCRIPTOR_COUNT; i++)
    {
        rx_descriptor[i] = (rx_desc *)((uint8_t *)descriptor + i * 16);
        rx_descriptor[i]->address = (uint64_t)(uint8_t *)(malloc(8192 + 16));
        rx_descriptor[i]->status = 0;
    }

    write(E_TX_DESCRIPTOR_LOW, (uint64_t)tmp_ptr >> 32);
    write(E_TX_DESCRIPTOR_HIGH, (uint64_t)tmp_ptr & 0xFFFFFFFF);
    write(E_RX_DESCRIPTOR_LOW, (uint64_t)tmp_ptr);
    write(E_RX_DESCRIPTOR_HIGH, 0);

    write(E_RX_DESCRIPTOR_LENGTH, RX_DESCRIPTOR_COUNT * 16);
    write(E_RX_DESCRIPTOR_HEAD, 0);
    write(E_RX_DESCRIPTOR_TAIL, RX_DESCRIPTOR_COUNT - 1);

    rx_current_buf = 0;
    write(E_RCONTROL, FOFF(ENABLE) | FOFF(STORE_BAD_PACKET) | FOFF(UNICAST_ENABLE) | FOFF(MULTICAST_ENABLE) | (0 << NO_LOOPBACK) | (0 << THRESHOLD) | FOFF(BROADCAST_ACCEPT) | (FOFF(STRIP_ETHERNET)) | RCONTROL_SIZE_8192);
}

void e1000::setup_tx()
{

    uint8_t *tmp_ptr;
    tx_desc *descriptor;
    tmp_ptr = (uint8_t *)malloc(sizeof(tx_desc) * TX_DESCRIPTOR_COUNT + 16);
    descriptor = (tx_desc *)tmp_ptr;
    for (int i = 0; i < TX_DESCRIPTOR_COUNT; i++)
    {
        tx_descriptor[i] = (tx_desc *)((uint8_t *)descriptor + i * 16);
        tx_descriptor[i]->address = 0;
        tx_descriptor[i]->status = DESC_DONE;
        tx_descriptor[i]->command = 0;
    }
    write(E_TX_DESCRIPTOR_LOW, (uint64_t)tmp_ptr >> 32);
    write(E_TX_DESCRIPTOR_HIGH, (uint64_t)tmp_ptr & 0xFFFFFFFF);

    write(E_TX_DESCRIPTOR_LENGTH, TX_DESCRIPTOR_COUNT * 16);

    write(E_TX_DESCRIPTOR_HEAD, 0);
    write(E_TX_DESCRIPTOR_TAIL, 0);

    tx_current_buf = 0;

    write(E_TCONTROL, TRANSMIT_ON | PAD_SHORT_PACKET | (15 << COLLISION_THRESHOLD) | (64 << COLLISION_DISTANCE) | RE_SEND_ON_LATE);
}

void e1000::turn_on_int()
{
    write(E_IMASK, 0x1f6dC);
    write(E_IMASK, 0xff & ~4);
    read(0xc0);
}

void e1000::init(pci_device *dev)
{

    log("Ne2000", LOG_DEBUG) << "loading Ne2000";
    pci_bar_data d = dev->get_bar(0);

    if (d.type == pci_bar_type::MM_IO_32)
    {
        mm_address = d.base;
    }
    else
    {
        io_base_addr = d.base;
    }
    if (eerp_rom_detection())
    {

        log("Ne2000", LOG_INFO) << "Ne2000 has eeprom";
    }
    log("Ne2000", LOG_INFO) << "mac address";

    mac_detection();
    for (int i = 0; i < 6; i++)
    {

        log("Ne2000", LOG_INFO) << i << " = " << get_mac_addr().mac[i];
    }
    setup_rx();
    setup_tx();

    turn_on_int();
}
