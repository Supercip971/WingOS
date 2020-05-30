#include <arch/gdt.h>
#include <kernel.h>
gdt_pointer gdt_ptr;
gdt_descriptor desc[4];
tss mtss;
extern "C" void gdt_flush();

int setup_gdt_64(uint8_t access, uint8_t Granularity, int gdt_id){
    gdt_descriptor* d = &desc[gdt_id];

    d->access_byte = access;
    d->base_high8 = 0;
    d->base_low16 = 0;
    d->base_mid8 = 0;
    d->Granularity = Granularity;
    d->limit_low16 = 0;
    return 0;
}uint64_t d_addr;
void setup_gdt(){
    gdt_ptr.base = (uint64_t)desc;
    gdt_ptr.limit =sizeof(desc) - 1;
    setup_gdt_64(0,0,0); // null
    setup_gdt_64(146,0,1); // data
    setup_gdt_64(154,32,2); // code
   
    asm volatile("lgdt [gdt_ptr]");
    
    gdt_flush();
}