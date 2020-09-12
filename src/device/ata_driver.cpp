#include <arch/arch.h>
#include <arch/mem/liballoc.h>
#include <com.h>
#include <device/ata_driver.h>
#include <loggging.h>
ata_driver main_driver;
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
    if (current_selected_drive == ATA_SECONDARY_MASTER || current_selected_drive == ATA_SECONDARY_MASTER)
    {
        is_primary = false;
    }
    while (true)
    {

        cur_status = ata_read(is_primary, ATA_reg_command_status);
        if ((cur_status & 0x80) == 0 && (cur_status & 0x8) != 0)
            return true;
    }
}
void ata_driver::init()
{
    waiting_for_irq = 0;
    log("ata", LOG_DEBUG) << "loading ata";
    current_selected_drive = ATA_PRIMARY_MASTER;

    outb(ATA_PRIMARY_DCR, 0x04);
    outb(ATA_PRIMARY_DCR, 0x00);
    uint8_t status = ata_read(true, ATA_reg_command_status);
    while (((status & 0x80) == 0x80) && ((status & 0x01) != 0x01))
    {
        status = ata_read(true, ATA_reg_command_status);
    }
    // reset
    uint8_t *temp_buffer = (uint8_t *)malloc(2048);
    for (int i = 0; i < 2048; i++)
    {
        temp_buffer[i] = 0;
    }

    read(0, 3, temp_buffer);
    log("ata", LOG_INFO) << "first bytes data : \n";
    for (int i = 0; i < 256 * 3; i++)
    {
        printf(" %c", temp_buffer[i]);
    }
    printf("\n");
    // init_drive(ATA_PRIMARY_MASTER);
    //  init_drive(ATA_PRIMARY_SLAVE);
    //  init_drive(ATA_SECONDARY_MASTER);
    //  init_drive(ATA_SECONDARY_SLAVE);
}

ata_driver *ata_driver::the()
{
    return &main_driver;
}
void ata_driver::irq_handle(uint64_t irq_handle_num)
{
    //log("ata", LOG_INFO) << "receive an ata interrupt";
    // printf("[ATA] ata driver receive an irq \n");
    if (waiting_for_irq == 1)
    {
        //     log("ata", LOG_INFO) << "receive an ata interrupt when waiting for it";
        //    printf("it was waiting for the interrupt \n");
        waiting_for_irq = 0;
    }
    if (irq_handle_num == 14)
    {
        //    printf("[ATA] it was the main slave \n");
        inb(0x1F7);
        return;
    }
    else if (irq_handle_num == 15)
    {
        //    printf("[ATA] it was the secondary slave \n");
        inb(0x177);
        return;
    }
    else
    {
        //    printf("[ERROR] ata driver receive not supported irq");
    }
}
void ata_driver::read(uint32_t where, uint8_t count, uint8_t *buffer)
{
    // printf("trying to read ata 0, in %x count %x to %x", where, count, (uint64_t)buffer);
    //log("ata", LOG_INFO) << "reading ata where : " << where << "count : " << count;
    waiting_for_irq = 1;
    ata_write(true, ATA_reg_error_feature, 0);
    ata_write(true, ATA_reg_selector, 0xE0 | 0x40 | ((where >> 24) & 0x0F));

    ata_write(true, ATA_reg_sector_count, count);
    ata_write(true, ATA_reg_lba0, (uint8_t)where);
    ata_write(true, ATA_reg_lba1, (uint8_t)(where >> 8));
    ata_write(true, ATA_reg_lba2, (uint8_t)(where >> 16));

    ata_write(true, ATA_reg_command_status, 0x20); // call the read command

    int new_count = count;
    uint32_t off = 0;
    uint64_t wait_time = 0;
    while (new_count-- > 0)
    {
        //  log("ata", LOG_INFO) << "ata waiting";
        uint8_t status = ata_read(true, ATA_reg_command_status);
        while (((status & 0x80) == 0x80) && ((status & 0x01) != 0x01))
        {
            status = ata_read(true, ATA_reg_command_status);
        }
        for (uint16_t i = 0; i < 256; i++)
        {

            *((unsigned short *)buffer + off + i) = inw(ATA_PRIMARY + ATA_reg_data);
        }
        off += 256;
    }
    return;
}
