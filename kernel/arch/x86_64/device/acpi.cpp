#include <device/acpi.h>
#include <device/apic.h>
#include <device/debug/com.h>
#include <logging.h>
#include <mem/virtual.h>
#include <utility.h>
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

void acpi::init()
{
    log("acpi", LOG_DEBUG, "loading acpi");

    descriptor = (RSDPDescriptor20 *)get_rsdp();
    rsdt_table = get_mem_addr<RSDT *>(descriptor->firstPart.RSDT_address);

    log("rsdt", LOG_DEBUG, "logging rsdt");
    RSDT *rsdt = get_mem_addr<RSDT *>((descriptor->firstPart.RSDT_address));
    int entries = (rsdt->h.Length - sizeof(rsdt->h)) / 4;

    for (int i = 0; i < entries; i++)
    {

        if (rsdt->PointerToOtherSDT[i] == 0)
        {
            continue;
        }

        RSDTHeader *h = get_mem_addr<RSDTHeader *>((rsdt->PointerToOtherSDT[i]));
        log("rsdt", LOG_INFO, "entry: {}, signature: {}, EOM: {} ", i, range_str(h->Signature, 4), range_str(h->OEMID, 6));
    }
}

void acpi::init_in_paging()
{

    RSDT *rsdt = rsdt_table;

    int entries = (rsdt->h.Length - sizeof(rsdt->h)) / 4;
    uintptr_t ddat = ALIGN_DOWN((uintptr_t)rsdt, PAGE_SIZE);

    map_page(ddat, get_mem_addr(ddat), true, false);
    map_page(ddat + PAGE_SIZE, get_mem_addr(ddat + PAGE_SIZE), true, false);

    rsdt = (RSDT *)get_mem_addr((uint64_t)rsdt);

    for (int i = 0; i < entries; i++)
    {
        uint64_t addr = rsdt->PointerToOtherSDT[i];
        addr = ALIGN_DOWN(addr, PAGE_SIZE);

        map_page(addr, get_mem_addr(addr), true, false);
        map_page(addr + PAGE_SIZE, get_mem_addr(addr + PAGE_SIZE), true, false);
        map_page(addr + (PAGE_SIZE * 2), get_mem_addr(addr + (PAGE_SIZE * 2)), true, false);
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
