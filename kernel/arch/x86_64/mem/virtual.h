#pragma once
#include <physical.h>

#include <stivale_struct.h>

#include <utils/attribute.h>
#include <utils/bit.h>
#include <utils/container/wvector.h>

#define KERNEL_PHYS_OFFSET ((uint64_t)0xffffffff80000000)
#define USR_MEM_OFFSET ((uint64_t)0x0000002000000000)
#define USR_BFRAME_SIZE ((uint64_t)0x0000000010000000)
#define MEM_PHYS_OFFSET ((uint64_t)0xffff800000000000)

#define PML4_GET_INDEX(addr) (addr & ((uint64_t)0x1ff << 39)) >> 39
#define PDPT_GET_INDEX(addr) (addr & ((uint64_t)0x1ff << 30)) >> 30
#define PAGE_DIR_GET_INDEX(addr) (addr & ((uint64_t)0x1ff << 21)) >> 21
#define PAGE_TABLE_GET_INDEX(addr) (addr & ((uint64_t)0x1ff << 12)) >> 12

#define FRAME_ADDR 0xfffffffffffff000

#define PAGE_SIZE 4096
#define TWO_MEGS (0x2000000)
#define FOUR_GIGS 0x100000000

enum class page_table_offset : size_t
{
    PRESENT_FLAG = 0,
    WRITABLE = 1,
    USER_ACCESSIBLE = 2,
    CACHING = 3,
    DISABLE_CACHE = 4,
    ACCESSED = 5,
    DIRTY = 6,
    HUGE_PAGE = 7,
    GLOBAL = 8,
    NO_EXECUTE = 63,
};

ASM_FUNCTION void set_paging();

class page_table
{
    uint64_t _raw;

public:
    constexpr page_table() : _raw(0){};
    constexpr page_table(uint64_t value) : _raw(value){};

    uint64_t raw() const { return _raw; };

    void set_raw(uint64_t new_value) { _raw = new_value; };

    constexpr bool is_present() const
    {
        return utils::get_bit(_raw, (size_t)page_table_offset::PRESENT_FLAG);
    };

    constexpr void set_present(bool new_value)
    {
        utils::set_bit(_raw, (size_t)page_table_offset::PRESENT_FLAG, new_value);
    };

    constexpr bool is_user() const
    {
        return utils::get_bit(_raw, (size_t)page_table_offset::USER_ACCESSIBLE);
    };

    constexpr void set_user(bool new_value)
    {
        utils::set_bit(_raw, (size_t)page_table_offset::USER_ACCESSIBLE, new_value);
    };

    constexpr bool is_writable() const
    {
        return utils::get_bit(_raw, (size_t)page_table_offset::WRITABLE);
    };

    constexpr void set_writable(bool new_value)
    {
        utils::set_bit(_raw, (size_t)page_table_offset::WRITABLE, new_value);
    };
    constexpr void set_noexecutable(bool new_value)
    {
        utils::set_bit(_raw, (size_t)page_table_offset::NO_EXECUTE, new_value);
    };

    uint64_t get_addr() const { return _raw & FRAME_ADDR; };

    static constexpr page_table create(uint64_t physical, bool is_writable, bool is_user)
    {
        page_table v = page_table(physical);

        v.set_present(true);
        v.set_user(is_user);
        v.set_writable(is_writable);

        return v;
    }

    static page_table *get_entry(page_table *table, uint64_t entry);

    static page_table *create_entry(page_table *table, uint64_t entry, bool is_writable, bool is_user);

    static page_table *get_or_create_entry(page_table *table, uint64_t entry, bool is_writable, bool is_user);
};

static_assert(sizeof(page_table) == sizeof(uint64_t), "page table must have the same size as a uint64_t");

inline void set_paging_dir(uint64_t pd)
{
    asm volatile("mov cr3, %0" ::"r"(pd & FRAME_ADDR));
}

void update_paging();

int map_page(uint64_t phys_addr, uint64_t virt_addr, bool is_writable, bool is_user);
int map_page(page_table *table, uint64_t phys_addr, uint64_t virt_addr, bool is_writable, bool is_user);

int unmap_page(page_table *table, uint64_t virt_addr);

uint64_t get_physical_addr(uint64_t virt);

page_table *new_vmm_page_dir();

void init_vmm(stivale2_struct_tag_memmap *bootdata);

page_table *clone_directory(page_table *from);
