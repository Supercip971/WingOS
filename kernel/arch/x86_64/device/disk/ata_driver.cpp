#include <arch.h>
#include <device/debug/com.h>
#include <device/disk/ata_driver.h>
#include <interrupt.h>
#include <logging.h>
#include <utils/lock.h>
#include <utils/memory/liballoc.h>

ata_driver main_driver;
utils::lock_type ata_lock;
int waiting_for_irq = 0;

ata_driver::ata_driver()
{
}

void ata_driver::ata_write(bool primary, uint16_t ata_register, uint16_t whattowrite)
{
    if (primary)
    {
        outb(ATA_PRIMARY + ata_register, whattowrite);
    }
    else
    {
        outb(ATA_SECONDARY + ata_register, whattowrite);
    }
}

uint8_t ata_driver::ata_read(bool primary, uint16_t ata_register)
{
    if (primary)
    {
        return inb(ATA_PRIMARY + ata_register);
    }
    else
    {
        return inb(ATA_SECONDARY + ata_register);
    }
}

bool ata_driver::get_ata_status()
{
    uint8_t cur_status;
    bool is_primary = true;
    while (true)
    {

        cur_status = ata_read(is_primary, ATA_reg_command_status);
        if ((cur_status & 0x80) == 0 && (cur_status & 0x8) != 0)
            return true;
    }
}

bool ata_driver::has_ata_device()
{
    bool is_primary = true;
    ata_write(is_primary, ATA_reg_sector_count, 0);
    ata_write(is_primary, ATA_reg_lba0, 0);
    ata_write(is_primary, ATA_reg_lba1, 0);
    ata_write(is_primary, ATA_reg_lba2, 0);
    ata_write(is_primary, ATA_reg_command_status, ATA_command::ATA_cmd_identify);

    uint8_t state = ata_read(is_primary, ATA_state_error);
    if (ata_read(is_primary, ATA_reg_command_status) == 0 || state != 0)
    {
        return false;
    }

    return true;
}

void ata_driver::init()
{
    waiting_for_irq = 0;
    log("ata", LOG_DEBUG, "loading ata");
    // get
    if (!has_ata_device())
    {
        log("ata", LOG_WARNING, "no ata device found for this system");
        return;
    }
    current_selected_drive = ATA_PRIMARY_MASTER;

    outb(ATA_PRIMARY_DCR, 0x04);
    outb(ATA_PRIMARY_DCR, 0x00);

    uint8_t status = ata_read(true, ATA_reg_command_status);
    while (((status & 0x80) == 0x80) && ((status & 0x01) != 0x01))
    {
        status = ata_read(true, ATA_reg_command_status);
    }
}

void ata_driver::irq_handle(uint64_t irq_handle_num)
{
    if (waiting_for_irq == 1)
    {
        waiting_for_irq = 0;
    }

    if (irq_handle_num == 14)
    {
        inb(0x1F7);
        return;
    }
    else if (irq_handle_num == 15)
    {
        inb(0x177);
        return;
    }
    else
    {
    }
}

void ata_driver::read(uint64_t where, uint32_t count, uint8_t *buffer)
{
    ata_lock.lock();
    waiting_for_irq = 1;
    ata_write(true, ATA_reg_selector, 0x40 | 0xE0);
    ata_write(true, ATA_reg_error_feature, 0);

    ata_write(true, ATA_reg_sector_count, count >> 8);
    ata_write(true, ATA_reg_lba0, (uint8_t)(where >> 24));
    ata_write(true, ATA_reg_lba1, (uint8_t)(where >> 32));
    ata_write(true, ATA_reg_lba2, (uint8_t)(where >> 40));

    ata_write(true, ATA_reg_sector_count, count);
    ata_write(true, ATA_reg_lba0, (uint8_t)where);
    ata_write(true, ATA_reg_lba1, (uint8_t)(where >> 8));
    ata_write(true, ATA_reg_lba2, (uint8_t)(where >> 16));

    ata_write(true, ATA_reg_command_status, ATA_command::ATA_cmd_read); // call the read command
    wait(5);
    uint32_t new_count = count;
    uint32_t off = 0;

    while (new_count-- > 0)
    {
        wait(5);
        uint8_t status = poll_status();

        if ((status & 0x1) || (status & 0x20))
        {
            wait(5);

            uint8_t error = ata_read(true, ATA_reg_error_feature);
            log("ata", LOG_ERROR, "ata error: {}", error);
            ata_lock.unlock();
            return;
        }
        for (uint16_t i = 0; i < 256; i++)
        {
            *((unsigned short *)buffer + (off + i)) = inw(ATA_PRIMARY + ATA_reg_data);
        }
        off += 256;
    }
    ata_lock.unlock();
    return;
}
