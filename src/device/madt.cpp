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

    while (uint64_t(table) < get_madt_table_lenght())
    {
        table = (MADT_record_table_entry *)(((uint64_t)table) + table->tlenght);
        com_write_str("madt table entry : ");
        com_write_reg("type = ", table->ttype);
        com_write_reg("lenght = ", table->tlenght);
        if (table->ttype == MADT_type::MADT_LAPIC)
        {

            auto local_apic = reinterpret_cast<MADT_table_LAPIC *>(table);

            com_write_str("MADT LAPIC INFO");
            com_write_reg("APIC id : ", local_apic->apic_id);
            com_write_reg("PROC id : ", local_apic->processor_id);
        }
    }
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
            MTIO[count] = local_apic;
            count++;
        }
    }
    MTIO[count] = 0;
    return nullptr;
}
madt *madt::the()
{
    return &main_madt;
}

MADT_record_table_entry *madt::get_madt_table_record()
{
    return madt_header->MADT_table;
}
