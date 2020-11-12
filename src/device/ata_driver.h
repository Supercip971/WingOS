#pragma once
#include <stdint.h>
enum ATA_IO
{
    ATA_PRIMARY = 0x1F0,
    ATA_SECONDARY = 0x170,
    ATA_PRIMARY_DCR = 0x3F6,
    ATA_SECONDARY_DCR = 0x376
};
enum ATA_drive_type
{
    ATA_PRIMARY_MASTER = 1,
    ATA_PRIMARY_SLAVE = 2,
    ATA_SECONDARY_MASTER = 3,
    ATA_SECONDARY_SLAVE = 4
};
enum ATA_register
{
    ATA_reg_data = 0,
    ATA_reg_error_feature = 1,
    ATA_reg_sector_count = 2,
    ATA_reg_lba0 = 3,
    ATA_reg_lba1 = 4,
    ATA_reg_lba2 = 5,
    ATA_reg_selector = 6,
    ATA_reg_command_status = 7,
    ATA_reg_sector_count2 = 8,
    ATA_reg_lba3 = 9,
    ATA_reg_lba4 = 10,
    ATA_reg_lba5 = 11,
    ATA_reg_control_status = 12,
    ATA_reg_dev_address = 13
};
enum ATA_data
{
    ATA_type_master = 0xA0,
    ATA_type_slave = 0xB0,
    ATA_state_busy = 0x80,
    ATA_state_sr_drq = 0x08,
    ATA_state_error = 0x01
};

class ata_driver
{
    ATA_drive_type current_selected_drive;
    void ata_write(bool primary, uint16_t ata_register, uint16_t whattowrite);
    uint8_t ata_read(bool primary, uint16_t ata_register);
    uint8_t ide_temp_buf[256];

public:
    ata_driver();
    inline void ata_write_primary(uint16_t ata_register, uint16_t whattowrit)
    {
        ata_write(true, ata_register, whattowrit);
    }
    inline void ata_write_secondary(uint16_t ata_register, uint16_t whattowrit)
    {
        ata_write(false, ata_register, whattowrit);
    }

    inline void wait(uint8_t time)
    {
        for (int i = 0; i < time; i++)
        {
            ata_read(true, ATA_reg_command_status);
        }
    }

    void read(uint32_t where, uint32_t count, uint8_t *buffer);
    bool get_ata_status();
    void irq_handle(uint64_t irq_handle_num);
    void init();
    static ata_driver *the();
};
