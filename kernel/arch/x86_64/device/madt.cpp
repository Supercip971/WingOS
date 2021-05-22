#include <device/acpi.h>
#include <device/debug/com.h>
#include <device/madt.h>
#include <logging.h>
#include <utils/memory/liballoc.h>
madt main_madt;

madt::madt()
{
}

uint64_t madt::get_madt_table_lenght()
{
    return ((uintptr_t)&madt_header->RShead) + madt_header->RShead.Length;
}

void madt::log_all()
{
    MADT_record_table_entry *table = madt_header->MADT_table;
    while (uintptr_t(table) < get_madt_table_lenght())
    {
        printf("\n ===================");
        table = (MADT_record_table_entry *)(((uint64_t)table) + table->tlenght);

        if (table->ttype == MADT_type::MADT_LAPIC)
        {
            log("madt", LOG_INFO, " madt LAPIC table entry : ");
            log("madt", LOG_INFO, "type   : {}", table->ttype);
            log("madt", LOG_INFO, "lenght : {}", table->tlenght);

            auto local_apic = reinterpret_cast<MADT_table_LAPIC *>(table);

            log("madt", LOG_INFO, " madt table LAPIC info : ");
            log("madt", LOG_INFO, "apic id : {}", local_apic->apic_id);
            log("madt", LOG_INFO, "proc id : {}", local_apic->processor_id);
        }
        else if (table->ttype == MADT_type::MADT_IOAPIC)
        {

            log("madt", LOG_INFO, " madt IOAPIC table entry : ");
            log("madt", LOG_INFO, "type   : {}", table->ttype);
            log("madt", LOG_INFO, "lenght : {}", table->tlenght);

            auto ioapic = reinterpret_cast<MADT_table_IOAPIC *>(table);

            log("madt", LOG_INFO, " madt table IOAPIC info : ");

            log("madt", LOG_INFO, "id             : {}", ioapic->IOAPIC_id);
            log("madt", LOG_INFO, "interrupt base : {}", ioapic->gsib);
            log("madt", LOG_INFO, "io apic addr   : {}", ioapic->ioapic_addr);
        }
        else if (table->ttype == MADT_type::MADT_LAPIC_OVERRIDE)
        {
            log("madt", LOG_INFO, " madt IOAPIC override table entry : ");
            log("madt", LOG_INFO, "type   : {}", table->ttype);
            log("madt", LOG_INFO, "lenght : {}", table->tlenght);

            auto ioapic = reinterpret_cast<MADT_table_LAPICO *>(table);

            log("madt", LOG_INFO, " madt table IOAPIC override info : ");
            log("madt", LOG_INFO, "new address : {}", ioapic->apic_address);
        }
        else if (table->ttype == MADT_type::MADT_ISO)
        {
            log("madt", LOG_INFO, " madt ISO table entry : ");
            log("madt", LOG_INFO, "type   : {}", table->ttype);
            log("madt", LOG_INFO, "lenght : {}", table->tlenght);

            auto iso = reinterpret_cast<MADT_table_ISO *>(table);

            log("madt", LOG_INFO, " madt table iso info : ");

            log("madt", LOG_INFO, "gsi       : {}", iso->interrupt);
            log("madt", LOG_INFO, "target    : {}", iso->irq);
            log("madt", LOG_INFO, "attribute : {}", iso->misc_flags);
        }
        else
        {

            log("madt", LOG_INFO, " not detected madt table entry : ");
            log("madt", LOG_INFO, "type   : {}", table->ttype);
            log("madt", LOG_INFO, "lenght : {}", table->tlenght);
        }
    }
}

void madt::init()
{
    log("madt", LOG_DEBUG, "loading madt");
    madt_address = acpi::the()->find_entry("APIC");

    madt_header = reinterpret_cast<MADT_head *>(madt_address);

    lapic_base = madt_header->lapic;

    uintptr_t lbase_addr = lapic_base;
    lbase_addr = ALIGN_UP(lbase_addr, PAGE_SIZE);

    map_page(lbase_addr, lbase_addr, true, false);
    map_page(lbase_addr + PAGE_SIZE, lbase_addr + PAGE_SIZE, true, false);

    log_all();
}

MADT_table_IOAPIC **madt::get_madt_ioAPIC()
{
    MADT_record_table_entry *table = madt_header->MADT_table;
    MADT_table_IOAPIC **MTIO = (MADT_table_IOAPIC **)malloc(255);
    uint64_t count = 0;

    while (uintptr_t(table) < get_madt_table_lenght())
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
    MADT_table_ISO **MTIO = reinterpret_cast<MADT_table_ISO **>(malloc(255));
    uint64_t count = 0;
    bool has_one = false;

    while (uintptr_t(table) < get_madt_table_lenght())
    {
        table = reinterpret_cast<MADT_record_table_entry *>(((uint64_t)table) + table->tlenght);

        if (table->ttype == MADT_type::MADT_ISO)
        {
            has_one = true;
            auto local_apic = reinterpret_cast<MADT_table_ISO *>(table);
            MTIO[count] = reinterpret_cast<MADT_table_ISO *>((uint64_t)local_apic);
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
