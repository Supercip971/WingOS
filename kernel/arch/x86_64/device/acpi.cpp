#include <com.h>
#include <device/acpi.h>
#include <device/apic.h>
#include <logging.h>
#include <utility.h>
#include <virtual.h>
acpi main_acpi;

void *get_rsdp(void)
{

    for (uintptr_t i = get_mem_addr(0x80000); i < get_mem_addr(0x100000); i += 16)
    {
        if (i == get_mem_addr(0xa0000))
        {
            i = get_mem_addr(0xe0000 - 16);
            continue;
        }

        if (!strncmp((char *)i, "RSD PTR ", 8))
        {
            return (void *)i;
        }
    }

    return nullptr;
}

acpi::acpi()
{
}

void *acpi::find_entry(const char *entry_name)
{

    RSDT *rsdt = get_mem_addr<RSDT *>((descriptor->firstPart.RSDT_address));
    int entries = (rsdt->h.Length - sizeof(rsdt->h)) / 4;

    for (int i = 0; i < entries; i++)
    {

        if (rsdt->PointerToOtherSDT[i] == 0)
        {
            continue;
        }

        RSDTHeader *h = get_mem_addr<RSDTHeader *>((rsdt->PointerToOtherSDT[i]));

        if (!strncmp(h->Signature, entry_name, 4))
        {
            return (void *)h;
        }
    }

    // Not found
    return nullptr;
}

void *findFACP(void *RootSDT)
{
    RSDT *rsdt = reinterpret_cast<RSDT *>(RootSDT);
    int entries = (rsdt->h.Length - sizeof(rsdt->h)) / 4;

    for (int i = 0; i < entries; i++)
    {
        if (rsdt->PointerToOtherSDT[i] == 0)
        {
            continue;
        }

        RSDTHeader *h = reinterpret_cast<RSDTHeader *>(rsdt->PointerToOtherSDT[i]);

        if (!strncmp(h->Signature, "FACP", 4))
        {
            return (void *)h;
        }
    }

    // No FACP found
    return nullptr;
}

void acpi::init(uint64_t rsdp)
{
    log("acpi", LOG_DEBUG) << "loading acpi";

    descriptor = (RSDPDescriptor20 *)get_rsdp();
    rsdt_table = get_mem_addr<RSDT *>(descriptor->firstPart.RSDT_address);
}

void acpi::init_in_paging()
{

    RSDT *rsdt = rsdt_table;

    int entries = (rsdt->h.Length - sizeof(rsdt->h)) / 4;
    uintptr_t ddat = ALIGN_DOWN((uintptr_t)rsdt, PAGE_SIZE);

    virt_map(ddat, get_mem_addr(ddat), 0x03);
    virt_map(ddat + PAGE_SIZE, get_mem_addr(ddat + PAGE_SIZE), 0x03);

    rsdt = (RSDT *)get_mem_addr((uint64_t)rsdt);

    for (int i = 0; i < entries; i++)
    {
        uint64_t addr = rsdt->PointerToOtherSDT[i];
        addr = ALIGN_DOWN(addr, PAGE_SIZE);

        virt_map(addr, get_mem_addr(addr), 0x03);
        virt_map(addr + PAGE_SIZE, get_mem_addr(addr + PAGE_SIZE), 0x03);
        virt_map(addr + (PAGE_SIZE * 2), get_mem_addr(addr + (PAGE_SIZE * 2)), 0x03);
    }
}

void acpi::getFACP()
{
    RSDT *rsdt = (RSDT *)rsdt_table;
    int entries = (rsdt->h.Length - sizeof(rsdt->h)) / 4;

    for (int i = 0; i < entries; i++)
    {

        if (rsdt->PointerToOtherSDT[i] == 0)
        {
            continue;
        }
    }
}

acpi *acpi::the()
{
    return &main_acpi;
}
