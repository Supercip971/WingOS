#pragma once

#include <kernel/generic/pmm.hpp>

#include "hw/mem/addr_space.hpp"

#include "math/align.hpp"

namespace arch::amd64
{
struct [[gnu::packed]] Page
{
    bool _present : 1;
    bool _writeable : 1;
    bool _user : 1;
    bool _write_through : 1;
    bool _cache_disabled : 1;
    bool _accessed : 1;
    bool _dirty : 1;
    bool _huge_page : 1;
    bool _global : 1;
    uint8_t _available : 3;

    uint64_t _address : 52;

    constexpr PhysAddr address() const { return PhysAddr(_address << 12); }
    constexpr void address(PhysAddr addr) { _address = addr._addr >> 12; }

    bool present() const { return _present; }
    void present(bool p) { _present = p; }

    bool writeable() const { return _writeable; }
    void writeable(bool w) { _writeable = w; }

    bool user() const { return _user; }
    void user(bool u) { _user = u; }

    bool write_through() const { return _write_through; }
    void write_through(bool wt) { _write_through = wt; }

    bool cache_disabled() const { return _cache_disabled; }
    void cache_disabled(bool cd) { _cache_disabled = cd; }

    bool accessed() const { return _accessed; }
    void accessed(bool a) { _accessed = a; }

    bool dirty() const { return _dirty; }
    void dirty(bool d) { _dirty = d; }

    bool huge_page() const { return _huge_page; }
    void huge_page(bool hp) { _huge_page = hp; }

    bool global() const { return _global; }
    void global(bool g) { _global = g; }
};

constexpr unsigned int PAGE_SIZE = 4096;
constexpr unsigned int PAGE_TABLE_SIZE = 512;

static_assert(sizeof(Page) == sizeof(uint64_t));

template <int level>
class [[gnu::packed]] PageTable
{
    Page entries[512];

public:
    using ChildTable = PageTable<level - 1>;

    static constexpr unsigned int index(VirtAddr addr)
    {
        return ((uint64_t)addr._addr & ((uint64_t)0x1ff << (12 + (level - 1) * 9))) >> (12 + (level - 1) * 9);
    }

    static constexpr unsigned int page_size()
    {
        return PAGE_SIZE << (level - 1);
    }

    Page &page(size_t index)
    {
        return entries[index];
    }

    Page &page_from_addr(VirtAddr addr)
    {
        return entries[index(addr)];
    }

    ChildTable *table(size_t index)
    {
        auto addr = entries[index].address();

        return toVirt(addr).as<ChildTable>();
    }

    ChildTable *table_from_addr(VirtAddr vaddr)
    {
        auto addr = page_from_addr(vaddr).address();

        return toVirt(addr).template as<ChildTable>();
    }

    void fill(Page page)
    {
        for (size_t i = 0; i < 512; i++)
        {
            entries[i] = page;
        }
    }

    void clear()
    {
        fill(Page{});
    }

    core::Result<void> map(VirtAddr vaddr, Page page)
    {
        if constexpr (level == 1)
        {
            page_from_addr(vaddr) = page;
        }
        else
        {
            auto &entry = page_from_addr(vaddr);
            if (!entry.present())
            {
                auto addr = (try$(Pmm::the().allocate(1, IOL_ALLOC_MEMORY_FLAG_NONE)));

                auto c = toVirt(addr).as<Page>();

                for (int i = 0; i < 512; i++)
                {
                    c[i] = Page{};
                }

                entry.present(true);
                entry.writeable(page._writeable);
                entry.user(page._user);
                entry.write_through(page._write_through);
                entry.cache_disabled(page._cache_disabled);
                entry.address(addr);
            }
            else
            {
                entry.writeable(page._writeable || entry.writeable());
                entry.user(page._user || entry.user());
                entry.write_through(page._write_through || entry.write_through());
                entry.cache_disabled(page._cache_disabled || entry.cache_disabled());
            }
            return table_from_addr(vaddr)->map(vaddr, page);
        }

        return {};
    }

    core::Result<void> unmap(VirtAddr vaddr, bool user)
    {
        if constexpr (level == 1)
        {
            page_from_addr(vaddr) = Page{};
        }
        else
        {
            auto &entry = page_from_addr(vaddr);
            if (!entry.present())
            {
                return "Page not present";
            }

            if(user && !entry.user())
            {
                return "Page not user accessible";
            }
            
            try$(table_from_addr(vaddr)->unmap(vaddr, user));
            for (int i = 0; i < 512; i++)
            {
                if (table_from_addr(vaddr)->page(i).present())
                {
                    return {};
                }
            }

            Pmm::the().release((entry.address()), 1);
            entry = Page{};
        }

        return {};
    }

    core::Result<void> _verify(VirtAddr addr)
    {
        auto &page = page_from_addr(addr);
        if (!page.present())
        {
            return "Page not present";
        }

        if (!page.user())
        {
            return "Page not user accessible";
        }

        if (!page.writeable())
        {
            return "Page is not writable";
        }

        if constexpr (level <= 1)
        {
            return core::Result<void>();
        }
        else
        {

            return table_from_addr(addr)->_verify(addr);
        }
    }

    core::Result<void> verify(VirtAddr vaddr, size_t size)
    {
        auto begin = math::alignDown(vaddr._addr, (size_t)PAGE_SIZE);
        auto end = math::alignUp(begin + size, (size_t)PAGE_SIZE);
        for (size_t i = begin; i < end; i += PAGE_SIZE)
        {
            auto addr = VirtAddr(i);
            try$(_verify(addr));
        }

        return {};
    }
};

static_assert(sizeof(PageTable<4>) == 4096);
static_assert(sizeof(PageTable<1>) == 4096);

using Pml5 = PageTable<5>;
using Pml4 = PageTable<4>;
using Pdpt = PageTable<3>;
using Pd = PageTable<2>;
using Pt = PageTable<1>;

} // namespace arch::amd64