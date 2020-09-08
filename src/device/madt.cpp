#include <arch/mem/liballoc.h>
#include <com.h>
#include <device/acpi.h>
#include <device/madt.h>
madt main_madt;
madt::madt()
{
}
uint64_t madt::get_madt_table_lenght()
{
    return ((uint64_t)&madt_header->RShead) + madt_header->RShead.Length;
}
void madt::init()
{
    madt_address = acpi::the()->find_entry("APIC");

    madt_header = (MADT_head *)madt_address;

    MADT_record_table_entry *table = madt_header->MADT_table;
    lapic_base = madt_header->lapic;
    while (uint64_t(table) < get_madt_table_lenght())
    {
        printf(" =================== \n");
        table = (MADT_record_table_entry *)(((uint64_t)table) + table->tlenght);
        if (table->ttype == MADT_type::MADT_LAPIC)
        {
            printf("madt LAPIC table entry :  \n");
            printf("type = %x \n", table->ttype);
            printf("lenght = %x \n", table->tlenght);

            auto local_apic = reinterpret_cast<MADT_table_LAPIC *>(table);

            printf("MADT LAPIC INFO \n");
            printf("APIC id : %x \n", local_apic->apic_id);
            printf("PROC id : %x \n", local_apic->processor_id);
        }
        else if (table->ttype == MADT_type::MADT_IOAPIC)
        {

            printf("madt IOAPIC entry :  \n");
            printf("type = %x \n", table->ttype);
            printf("lenght = %x \n", table->tlenght);
            auto ioapic = reinterpret_cast<MADT_table_IOAPIC *>(table);

            printf("MADT IOAPIC INFO \n");
            printf("id : %x \n", ioapic->IOAPIC_id);
            printf("interrupt base : %x \n", ioapic->gsib);
            printf("io apic addr : %x \n", ioapic->ioapic_addr);
        }
        else if (table->ttype == MADT_type::MADT_LAPIC_OVERRIDE)
        {
            printf("madt IOAPIC override entry :  \n");
            printf("type = %x \n", table->ttype);
            printf("lenght = %x \n", table->tlenght);
            auto ioapic = reinterpret_cast<MADT_table_LAPICO *>(table);

            printf("MADT ioapic override INFO \n");
            printf("new address %x \n", ioapic->apic_address);
        }
        else if (table->ttype == MADT_type::MADT_ISO)
        {
            printf("madt ISO entry :  \n");
            printf("type = %x \n", table->ttype);
            printf("lenght = %x \n", table->tlenght);
            auto iso = reinterpret_cast<MADT_table_ISO *>(table);

            printf("MADT ioapic override INFO \n");
            printf("gsi %x \n", iso->interrupt);
            printf("target %x \n", iso->irq);
            printf("attribute %x \n", iso->misc_flags);
        }
        else
        {

            printf("madt table entry :  \n");
            printf("type = %x \n", table->ttype);
            printf("lenght = %x \n", table->tlenght);
        }
    }

    uint64_t lbase_addr = lapic_base;
    lbase_addr /= 4096;
    lbase_addr *= 4096;
    virt_map(lbase_addr, get_mem_addr(lbase_addr), 0x03);
    virt_map(lbase_addr + 4096, get_mem_addr(lbase_addr) + 4096, 0x03);

    lapic_base = get_mem_addr(lapic_base);
}

MADT_table_IOAPIC **madt::get_madt_ioAPIC()
{
    MADT_record_table_entry *table = madt_header->MADT_table;
    MADT_table_IOAPIC **MTIO = (MADT_table_IOAPIC **)malloc(255);
    uint64_t count = 0;
    while (uint64_t(table) < get_madt_table_lenght())
    {
        table = (MADT_record_table_entry *)(((uint64_t)table) + table->tlenght);

        if (table->ttype == MADT_type::MADT_IOAPIC)
        {

            auto local_apic = reinterpret_cast<MADT_table_IOAPIC *>(table);
            MTIO[count] = (MADT_table_IOAPIC *)((uint64_t)local_apic);
            count++;
        }
    }
    MTIO[count] = 0;
    return MTIO;
}
MADT_table_ISO **madt::get_madt_ISO()
{
    MADT_record_table_entry *table = madt_header->MADT_table;
    MADT_table_ISO **MTIO = (MADT_table_ISO **)malloc(255);
    uint64_t count = 0;
    bool has_one = false;
    while (uint64_t(table) < get_madt_table_lenght())
    {
        table = (MADT_record_table_entry *)(((uint64_t)table) + table->tlenght);

        if (table->ttype == MADT_type::MADT_ISO)
        {
            has_one = true;
            auto local_apic = reinterpret_cast<MADT_table_ISO *>(table);
            MTIO[count] = (MADT_table_ISO *)((uint64_t)local_apic);
            count++;
        }
    }
    if (has_one == false)
    {
        free(MTIO);
        return nullptr;
    }
    MTIO[count] = 0;
    return MTIO;
}
madt *madt::the()
{
    return &main_madt;
}

MADT_record_table_entry *madt::get_madt_table_record()
{
    return madt_header->MADT_table;
}
