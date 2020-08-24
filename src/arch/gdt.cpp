#include <arch/gdt.h>
#include <kernel.h>
#include <stddef.h>
#pragma GCC optimize ("-O0")
/* flags */
#define GDT_CS       0x18
#define GDT_DS       0x10
#define GDT_TSS      0x09
#define GDT_WRITABLE 0x02
#define GDT_USER     0x60
#define GDT_PRESENT  0x80

/* granularity */
#define GDT_LM       0x2
gdtr_t gdtr;



 gdt_descriptor_t gdt_descriptors[64];
  tss_t tss ; __attribute__((aligned(4096)))
static void gdt_set_descriptor(gdt_descriptor_t *gdt_descriptors, uint16_t sel, uint8_t flags, uint8_t gran)
{
  gdt_descriptor_t *descriptor = &gdt_descriptors[sel / sizeof(*gdt_descriptors)];
  descriptor->flags = flags;
  descriptor->granularity = (gran << 4) | 0x0F;
  descriptor->limit_low = 0xFFFF;
}

static void gdt_set_xdescriptor(gdt_descriptor_t *gdt_descriptors, uint16_t sel, uint8_t flags, uint8_t gran, uint64_t base, uint64_t limit)
{
  gdt_xdescriptor_t *descriptor = (gdt_xdescriptor_t *) (&gdt_descriptors[sel / sizeof(*gdt_descriptors)]);
  descriptor->low.flags = flags;
  descriptor->low.granularity = (gran << 4) | ((limit >> 16) & 0x0F);
  descriptor->low.limit_low = limit & 0xFFFF;
  descriptor->low.base_low = base & 0xFFFF;
  descriptor->low.base_mid = ((base >> 16) & 0xFF);
  descriptor->low.base_high = ((base >> 24) & 0xFF);
  descriptor->high.base_xhigh = ((base >> 32) & 0xFFFFFFFF);
  descriptor->high.reserved = 0;
}

extern "C" void gdtr_install(gdtr_t*, unsigned short, unsigned short);
void rgdt_init(void)
{
  /* get this CPU's local data */

  /* get pointers to the GDT and GDTR */

  /* get pointer to the TSS and calculate the limit */
  uint64_t tss_base = (uint64_t) &tss;
  uint64_t tss_limit =tss_base+ sizeof(tss)-1;

  /* reset the GDT */
  memzero(&gdt_descriptors,sizeof(gdt_descriptors) * 64);

  /* fill in the entries we need */
  gdt_set_descriptor( gdt_descriptors, SLTR_KERNEL_CODE, GDT_PRESENT | GDT_CS,                           GDT_LM);
  gdt_set_descriptor( gdt_descriptors, SLTR_KERNEL_DATA, GDT_PRESENT | GDT_DS | GDT_WRITABLE,            0);
  gdt_set_descriptor( gdt_descriptors, SLTR_USER_DATA,   GDT_PRESENT | GDT_DS | GDT_USER | GDT_WRITABLE, 0);
  gdt_set_descriptor( gdt_descriptors, SLTR_USER_CODE,   GDT_PRESENT | GDT_CS | GDT_USER,                GDT_LM);
  gdt_set_xdescriptor(gdt_descriptors, SLTR_TSS,         GDT_PRESENT | GDT_TSS,                          0, tss_base, tss_limit);

  /*
   * read the GS_BASE MSRs so we can restore it after updating the segment
   * registers
   */

  /* update the GDTR structure and install it */
  gdtr.addr = (uint64_t) &gdt_descriptors;
  gdtr.len = sizeof(gdt_descriptors) * GDT_DESCRIPTORS - 1;
  gdtr_install(&gdtr, SLTR_KERNEL_CODE, SLTR_KERNEL_DATA);

  /* restore the GS_BASE and GS_KERNEL_BASE MSR */
 
}void rgdt_init(void);

extern "C" void tss_install(uint16_t selector);

uint8_t idt_stack[4096*4]__attribute__((aligned(4096)));
uint8_t idt_stack2[4096*4]__attribute__((aligned(4096)));
uint8_t idt_stack3[4096*4]__attribute__((aligned(4096)));
void tss_init(uint64_t i)
{
  /* find this CPU's TSS */

  /* reset all the fields */
  memzero(&tss, sizeof(tss));
  tss.iomap_base = sizeof(tss)-1;
    tss.rsp0 =  (uint64_t)idt_stack+4096*4 - 32;
    tss.rsp1 = (uint64_t)idt_stack2+4096*4 - 32;
    tss.rsp2 = (uint64_t)idt_stack3+4096*4 - 32;
    
  /* install it using the LTR instruction */

    asm volatile("mov ax, %0 \n ltr ax" : : "i" (SLTR_TSS) : "rax");
}

void tss_set_rsp0(uint64_t rsp0)
{
  /* find this CPU's TSS */

  /* set the stack pointer for this CPU */
  tss.rsp0 = rsp0;
}void setup_gdt(unsigned long i){
    rgdt_init();
}
