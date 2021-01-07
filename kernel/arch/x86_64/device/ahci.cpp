#include "ahci.h"
#include <arch.h>
#include <io_device.h>
#include <liballoc.h>
#include <logging.h>
#include <utility.h>
lock_type ahci_lock;
ahci main_ahci;
ahci *ahci::the()
{
    return &main_ahci;
}

void ahci::init(pci_device *dev, uint8_t func)
{
    log("ahci", LOG_DEBUG) << "loading ahci...";
    ahci_bar = dev->get_bar(5, func);
    log("ahci", LOG_INFO) << "ahci addr : " << ahci_bar.base;
    data_addr = ahci_bar.base;
    hba_mem = (hba_memory *)data_addr;
    log("ahci", LOG_INFO) << "implemented register value = " << hba_mem->port_implemented;
    uint32_t dev_target = hba_mem->port_implemented;
    for (int i = 0; i < 32; i++)
    {
        if (dev_target & 1)
        {
            int dev_type = check_device_type(&hba_mem->ports[i]);
            if (dev_type != NULL_PORT)
            {
                reinit_port(&hba_mem->ports[i]);
            }
            if (dev_type == SATAPI_PORT)
            {
                log("ahci", LOG_INFO) << "satapi port at" << i;
            }
            else if (dev_type == SATA_PORT)
            {
                ahci_ata_device *ahci_device = new ahci_ata_device();
                ahci_device->set_port(&hba_mem->ports[i]);
                add_io_device((ahci_device));

                log("ahci", LOG_INFO) << "sata port at" << i;
            }
            else if (dev_type == SEMB_PORT)
            {
                log("ahci", LOG_INFO) << "semb port at" << i;
            }
            else if (dev_type == PM_PORT)
            {
                log("ahci", LOG_INFO) << "pm port at" << i;
            }
            else
            {
                log("ahci", LOG_INFO) << "null port at" << i;
            }
        }
        dev_target >>= 1;
    }
}

ahci_port_type ahci::check_device_type(hba_port *port) const
{
    uint32_t ssts = port->sata_status;
    uint8_t ipm = (ssts >> 8) & 0x0f;
    uint8_t det = ssts & 0x0f;
    if (det != HBA_PORT_DET_PRESENT)
    {
        return NULL_PORT;
    }
    if (ipm != HBA_PORT_IPM_ACTIVE)
    {
        return NULL_PORT;
    }
    if (port->signature == SATA_SIG_ATAPI)
    {
        return SATAPI_PORT;
    }
    else if (port->signature == SATA_SIG_SEMB)
    {
        return SEMB_PORT;
    }
    else if (port->signature == SATA_SIG_PM)
    {
        return PM_PORT;
    }
    else
    {
        return SATA_PORT;
    }
}
void ahci::reinit_port(hba_port *port)
{

    void *data = pmm_alloc(1);
    memzero(data, 4096);
    port->command_list_base_addr_low = (uint64_t)data;
    port->fis_base_addr_up = 0; // i don't think the kernel will eat 4g of memory

    hba_fis *fis = new hba_fis;
    memzero(fis, sizeof(hba_fis));

    fis->fsfis.type = fis_type_def::FTYPE_DMA_SETUP;
    fis->rfis.type = fis_type_def::FTYPE_REG_DEVICE2HOST;
    fis->psfis.type = fis_type_def::FTYPE_PIO_SETUP;

    port->fis_base_addr_low = (uint64_t)(fis);
    port->fis_base_addr_up = 0;

    volatile hba_cmd_header *command_header = (hba_cmd_header *)fis;
    for (int i = 0; i < 32; i++)
    {
        command_header[i].physical_region_descriptor_table_entry_count = 8;
        command_header[i].command_table_desc_base_addr = (uint64_t)pmm_alloc(1);
        command_header[i].command_table_desc_base_addr_up = 0;
    }
}
void ahci::start_command(hba_port *port)
{
    port->command_and_status &= ~COMMAND_ST;
    while (port->command_and_status & COMMAND_CR)
    {
    }
    port->command_and_status |= COMMAND_FRE;
    port->command_and_status |= COMMAND_ST;
}
void ahci::end_command(hba_port *port)
{
    port->command_and_status &= ~COMMAND_ST;
    while (port->command_and_status & COMMAND_CR)
    {
    }
    port->command_and_status &= ~COMMAND_FRE;
}
void ahci_ata_device::start_command()
{
    hba_port *nport = (hba_port *)port;

    nport->command_and_status &= ~COMMAND_ST;
    while (port->command_and_status & COMMAND_CR)
    {
    }
    nport->command_and_status |= COMMAND_FRE;
    nport->command_and_status |= COMMAND_ST;
}
void ahci_ata_device::end_command()
{
    hba_port *nport = (hba_port *)port;

    nport->command_and_status &= ~COMMAND_ST;
    while (port->command_and_status & COMMAND_CR)
    {
    }
    nport->command_and_status &= ~COMMAND_FRE;
}
io_rw_output ahci_ata_device::read(uint8_t *data, uint64_t count, uint64_t cursor)
{
    flock(&ahci_lock);
    uint16_t *rdata = (uint16_t *)data;
    port->interrupt_status = (uint32_t)-1;
    int spin_timeout = 0;
    int slot = find_command_slot();
    if (slot == -1)
    {
        return io_rw_output::io_ERROR;
    }
    volatile hba_cmd_header *command_header = (hba_cmd_header *)(uint64_t)port->command_list_base_addr_low;
    // fill command header
    command_header += slot;
    command_header->command_fis_length = sizeof(fis_reg_host2device) / sizeof(uint32_t);
    command_header->write = 0;
    command_header->physical_region_descriptor_table_entry_count = 1;
    // fill command tables
    volatile hba_command_table *command_table = (hba_command_table *)(uint64_t)command_header->command_table_desc_base_addr;
    memzero((hba_command_table *)command_table, sizeof(hba_command_table) + command_header->physical_region_descriptor_table_entry_count - 1 * sizeof(hba_prdt_entry));

    command_table->entry[0].data_base_addr_low = (uint64_t)rdata;
    command_table->entry[0].data_base_addr_up = (uint64_t)(rdata) >> 32;
    command_table->entry[0].interrupt_on_complete = 1;
    command_table->entry[0].dbc = (count * 512) - 1;
    // fill setup command
    volatile fis_reg_host2device *command = (fis_reg_host2device *)(command_table->command_fis);
    memzero((fis_reg_host2device *)command, sizeof(fis_reg_host2device));
    command->type = FTYPE_REG_HOST2DEVICE;
    command->cmd_or_ctrl = 1;
    command->command_register = DMA_ATA_READ_EXTENDED;
    command->lba0 = (uint8_t)((cursor & 0x000000000000ff));
    command->lba1 = (uint8_t)((cursor & 0x0000000000ff00) >> 8);
    command->lba2 = (uint8_t)((cursor & 0x00000000ff0000) >> 16);
    command->device = 1 << 6;
    command->lba3 = (uint8_t)((cursor & 0x000000ff000000) >> 24);
    command->lba4 = (uint8_t)((cursor & 0x0000ff00000000) >> 32);
    command->lba5 = (uint8_t)((cursor & 0x00ff0000000000) >> 40);
    command->count_low = count & 0xff;
    command->count_high = (count >> 8) & 0xff;

    while ((port->task_file_data & (ata_device_status::ATA_DEV_BUSY | ata_device_status::ATA_DEV_DRQ)) && spin_timeout < 1000000)
    {
        spin_timeout++;
    }
    if (spin_timeout == 1000000)
    {
        log("ahci", LOG_ERROR) << "ata device timed out";
        return io_rw_output::io_ERROR;
    }
    start_command();
    port->command_issue = 1 << slot;
    while (true)
    {
        if (!(port->command_issue & (1 << slot)))
        {
            break;
        }
        if (port->interrupt_status & TASK_FILE_ERROR)
        {
            log("ahci", LOG_ERROR) << "ata device task file error";
            return io_rw_output::io_ERROR;
        }
    }
    if (port->interrupt_status & TASK_FILE_ERROR)
    {
        log("ahci", LOG_ERROR) << "ata device task file error";
        return io_rw_output::io_ERROR;
    }
    end_command();
    unlock(&ahci_lock);
    return io_rw_output::io_OK;
}
io_rw_output ahci_ata_device::write(uint8_t *data, uint64_t count, uint64_t cursor)
{

    return io_rw_output::io_OK;
}
int ahci_ata_device::find_command_slot()
{
    uint32_t ata_slots = (port->sata_active | port->command_issue);
    for (uint32_t i = 0; i < 32; i++)
    {
        if ((ata_slots & 1) == 0)
        {

            return i;
        }
        ata_slots >>= 1;
    }
    log("ahci", LOG_ERROR) << "can't find free command slot";
    return -1;
}
