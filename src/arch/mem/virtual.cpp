#include <arch/mem/virtual.h>

#include <arch/arch.h>
#include <kernel.h>
#include <com.h>
#include <utility.h>

extern "C" uint64_t kernel_end;
uint32_t* frames = 0;
void init_frame(uint64_t lenght){
    lenght = 4096 * 4096 * 4096;
  frames = (uint32_t*)&kernel_end;
}
paging_pml4 pml4e[512] __attribute__((aligned(4096)));
paging_pdpe  pdpe[512] __attribute__((aligned(4096)));
paging_pde   pde[512] __attribute__((aligned(4096)));
uint64_t get_kern_addr(uint64_t addr){
    return addr + 0xffffffff80000000;
}uint64_t get_r_addr(uint64_t addr){
    return addr - 0xffffffff80000000;
}
#define PAGE_FRAME 0xFFFFFFFFFF000

inline void SetPageFrame(uint64_t* page, uint64_t addr){
        *page = (*page & ~PAGE_FRAME) | (addr & PAGE_FRAME);
    }

inline void SetPageFlags(uint64_t* page, uint64_t flags){
   *page |= flags;
}
void init_virtual_memory(stivale_struct* sti_struct){
    e820_entry_t* mementry =(e820_entry_t*) sti_struct->memory_map_addr;
    char buffer[64];
    memzero(buffer, sizeof(buffer));
    for (int i = 0; i < sti_struct->memory_map_entries; i++)
    {
        if (mementry[i].type != MEMMAP_USABLE)
        {
            continue;
        }
        
        com_write_str(" ============== ");
        kitoaT<uint64_t>(buffer, 'x', mementry[i].base);
        com_write_str(" memory start : ");
        com_write_str(buffer);
        memzero(buffer, sizeof(buffer));
         kitoaT<uint64_t>(buffer, 'x', mementry[i].length + mementry[i].base);
        com_write_str(" memory end : ");
        com_write_str(buffer);
        memzero(buffer, sizeof(buffer));
        kitoaT<uint64_t>(buffer, 'x', mementry[i].length );
        com_write_str(" memory lenght : ");
        com_write_str(buffer);
        memzero(buffer, sizeof(buffer));
        kitoaT<uint32_t>(buffer, 'x', mementry[i].type);
        com_write_str(" memory type : ");
        com_write_str(buffer);
        memzero(buffer, sizeof(buffer));
        
    }
    
/*    memzero(pml4e, sizeof(pml4e));
    memzero(pdpe, sizeof(pdpe));
    memzero(pde, sizeof(pde));


    for (int i = 256; i < 512; i++)
    {
        pml4e[i].as_uint = get_r_addr((uint64_t)&pml4e[i]);
        pml4e[i].fields.present = 1;
        pml4e[i].fields.writable = 1;
    }
    */


}



void virt_map(uint64_t vaddress, uint64_t paddress, uint64_t flags ){
    
	uint64_t _pml4e_offset = PML4_GET_INDEX(vaddress);
	uint64_t _pdpt_offset  = PDPT_GET_INDEX(vaddress);
	uint64_t _pd_offset   = PAGE_DIR_GET_INDEX(vaddress);
	uint64_t _pt_offset = PAGE_TABLE_GET_INDEX(vaddress);
    
    if (!pml4e[_pml4e_offset].fields.present)
    {
    }
    
}