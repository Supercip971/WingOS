#include <arch/arch.h>
#define RMFLAGS 0x000FFFFFFFFFF000

#define PML4_GET_INDEX(addr) (((addr) >> 39) & 0x1FF)
#define PDPT_GET_INDEX(addr) (((addr) >> 30) & 0x1FF)
#define PAGE_DIR_GET_INDEX(addr) (((addr) >> 21) & 0x1FF)
#define PAGE_TABLE_GET_INDEX(addr) (((addr) >> 12) & 0x1FF)
union paging_pml4{
    uint64_t as_uint;
    struct {
        uint64_t present : 1; // if the page is present
        uint64_t writable : 1; // if the page is writable 
        uint64_t user : 1; // for ring 3 process
        uint64_t write_through : 1;
        uint64_t disable_caching : 1;
        uint64_t accessed : 1;
        uint64_t ignore_me_please : 1;
        uint64_t zero : 1;
        uint64_t zero2 : 1;
        uint64_t not_used : 3;
        uint64_t adress : 40;
        uint64_t not_used2 : 11;
        uint64_t frame_adress : 1; 
    }fields;
}__attribute__((packed));


union paging_pdpe{

    uint64_t as_uint;
    struct {
        uint64_t present : 1; // if the page is present
        uint64_t writable : 1; // if the page is writable 
        uint64_t user : 1; // for ring 3 process
        uint64_t write_through : 1;
        uint64_t disable_caching : 1;
        uint64_t accessed : 1;
        uint64_t ignore_me_please : 1;
        uint64_t zero : 1;
        uint64_t ignore_me_please2 : 1;
        uint64_t not_used : 3;
        uint64_t adress : 40;
        uint64_t not_used2 : 11;
        uint64_t frame_adress : 1; 
    }fields;
}__attribute__((packed));

union paging_pde{

    uint64_t as_uint;
    struct {
        uint64_t present : 1; // if the page is present
        uint64_t writable : 1; // if the page is writable 
        uint64_t user : 1; // for ring 3 process
        uint64_t write_through : 1;
        uint64_t disable_caching : 1;
        uint64_t accessed : 1;
        uint64_t ignore_me_please : 1;
        uint64_t one : 1;
        uint64_t global : 1;
        uint64_t available : 3;
        uint64_t pat : 1; // PAT translation mode
        uint64_t zero : 8;
        uint64_t adress : 31;
        uint64_t not_used2 : 11;
        uint64_t frame_adress : 1; 
    }fields;
}__attribute__((packed));
#include <stivale.h>
void init_virtual_memory(stivale_struct* sti_struct);