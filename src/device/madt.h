#pragma once
#include <device/acpi.h>



struct MADT_record_table_entry{
    uint8_t ttype;
    uint8_t tlenght;

};

struct MADT_head{
    RSDTHeader RShead;

    uint32_t lapic;
    uint32_t flags;

}__attribute__((packed));



class madt
{
    void* madt_address = 0x0;
public:
    madt();
    void init();
    static madt* the();
};

