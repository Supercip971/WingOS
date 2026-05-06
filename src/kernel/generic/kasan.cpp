
#include "kasan.hpp"
#include "kernel/generic/cpu.hpp"
#include "kernel/generic/paging.hpp"
#include "kernel/generic/pmm.hpp"
#include "libcore/alloc/alloc.hpp"
#include "libcore/fmt/flags.hpp"
#include "mcx/mcx.hpp"
#include <libcore/fmt/log.hpp>
#include <string.h>
#include "hw/mem/addr_space.hpp"

static kernel::Kasan _instance = {};



#define no_kasan __attribute__((no_sanitize("address")))
// source: https://github.com/FireflyOS/Firefly-Kernel/blob/master/firefly/kernel/trace/sanitizer/kasan.cpp#L88


core::Result<void> kernel::Kasan::initialize(size_t memory_size)
{
    _instance._shadow_memory_size = memory_size / 8;
    _instance._shadow_memory = (uint8_t*)try$(core::mem_alloc(_instance._shadow_memory_size));
    memset(_instance._shadow_memory, KasanTags::KASAN_UNAVAILABLE, _instance._shadow_memory_size);
    _instance.enable();
    return {};
}


//#error https://github.com/BlueGummi/charmos/blob/458e425094660e66274045a93a53f457b132a3b7/kernel/mem/asan.c#L275

void kernel::Kasan::preload(mcx::MachineContext* ctx)
{
    for(int i = 0; i < ctx->_memory_map_count; i++)
    {
        auto& region = ctx->_memory_map[i];
        switch(region.type)
        {
            case mcx::MemoryMap::Type::FREE:
                this->set_region_tag(toVirtRange(region.range.as<PhysAddr>()), KasanTags::KASAN_TAG_FREE);
                break;
            case mcx::MemoryMap::Type::KERNEL_AND_MODULES:
                this->set_region_tag(toVirtRange(region.range.as<PhysAddr>()), KasanTags::KASAN_TAG_KERNEL);
                break;

            case mcx::MemoryMap::Type::RESERVED:
            case mcx::MemoryMap::Type::ACPI_RECLAIMABLE:
            case mcx::MemoryMap::Type::ACPI_NVS:
            case mcx::MemoryMap::Type::BAD_MEMORY:
            case mcx::MemoryMap::Type::BOOTLOADER_RECLAIMABLE:
            case mcx::MemoryMap::Type::FRAMEBUFFER:
                this->set_region_tag(toVirtRange(region.range.as<PhysAddr>()), KasanTags::KASAN_TAG_KERNEL);
                break;
            default:
                break;
        }
    }
}



inline void kernel::Kasan::set_region_tag(VirtRange range, KasanTags tag)
{

    if(_shadow_memory == nullptr)
        return;

    uintptr_t shadow_start = toPhys(range.start()) / 8;
    uintptr_t shadow_end = toPhys(range.end()) / 8;

    for (uintptr_t shadow_addr = shadow_start; shadow_addr <= shadow_end; shadow_addr++)
    {
        _shadow_memory[shadow_addr] = static_cast<uint8_t>(tag);
    }
}



inline void kernel::Kasan::assert_read_access(uintptr_t addr, size_t size)
{

    if(!_enabled)
        return;


    this->_lock.lock();
    _enabled = false;

    uintptr_t shadow_start = addr / 8;
    uintptr_t shadow_end = (addr + size - 1) / 8;


    uintptr_t addr_tag = (addr >> 56) & 0xFF;

    if(addr_tag == 0)
    {
        // Skip the check for addresses with tag 0
        _enabled = true;
        this->_lock.release();
        return;
    }
    for (uintptr_t shadow_addr = shadow_start; shadow_addr <= shadow_end; shadow_addr++)
    {
        uint8_t shadow_value = _shadow_memory[shadow_addr];

        // Calculate the first invalid byte within this shadow byte
        uintptr_t first_invalid_byte = shadow_addr * 8;
        if (first_invalid_byte < addr + size && (shadow_value == 0 || shadow_value == 0x1))
        {
            fmt::err$("kasan: invalid read access at address {}, size {}\n", addr | fmt::FMT_HEX, size);

            unreachable$();
        }
    }

    _enabled = true;

    this->_lock.release();
}


inline void kernel::Kasan::assert_write_access(uintptr_t addr, size_t size)
{
    if(!_enabled)
        return;

    _enabled = false;

    uintptr_t shadow_start = addr / 8;
    uintptr_t shadow_end = (addr + size - 1) / 8;

    for (uintptr_t shadow_addr = shadow_start; shadow_addr <= shadow_end; shadow_addr++)
    {
        uint8_t shadow_value = _shadow_memory[shadow_addr];

        // Calculate the first invalid byte within this shadow byte
        uintptr_t first_invalid_byte = shadow_addr * 8;
        if (first_invalid_byte < addr + size && (shadow_value == 0 || shadow_value == 0x1))
        {
            fmt::err$("kasan: invalid write access at address {}, size {}\n", addr | fmt::FMT_HEX, size);

            unreachable$();
        }
    }

    _enabled = true;

    this->_lock.release();
}


extern "C" void __asan_before_dynamic_init()
{
}

extern "C" void __asan_after_dynamic_init()
{
}

extern "C" void __asan_loadN_noabort(void* p, size_t size)
{
    _instance.assert_read_access(reinterpret_cast<uintptr_t>(p), size);
}



extern "C" void __asan_storeN_noabort(void* p, int size)
{
    _instance.assert_write_access(reinterpret_cast<uintptr_t>(p), size);
}


extern "C" void __asan_load1_noabort(void* p)
{
    _instance.assert_read_access(reinterpret_cast<uintptr_t>(p), 1);
}

extern "C" void __asan_load2_noabort(void* p)
{
    _instance.assert_read_access(reinterpret_cast<uintptr_t>(p), 2);
}

extern "C" void __asan_load4_noabort(void* p)
{
    _instance.assert_read_access(reinterpret_cast<uintptr_t>(p), 4);
}

extern "C" void __asan_load8_noabort(void* p)
{
    _instance.assert_read_access(reinterpret_cast<uintptr_t>(p), 8);
}

extern "C" void __asan_load16_noabort(void* p)
{
    _instance.assert_read_access(reinterpret_cast<uintptr_t>(p), 16);
}

extern "C" void __asan_store1_noabort(void* p)
{
    _instance.assert_write_access(reinterpret_cast<uintptr_t>(p), 1);
}


extern "C" void __asan_store2_noabort(void* p)  {
    _instance.assert_write_access(reinterpret_cast<uintptr_t>(p), 2);
}


extern "C" void __asan_store4_noabort(void* p)  {
    _instance.assert_write_access(reinterpret_cast<uintptr_t>(p), 4);
}


extern "C" void __asan_store8_noabort(void* p)  {
    _instance.assert_write_access(reinterpret_cast<uintptr_t>(p), 8);
}

extern "C" void __asan_store16_noabort(void* p)  {
    _instance.assert_write_access(reinterpret_cast<uintptr_t>(p), 16);
}


extern "C" void __asan_handle_no_return()  {
    // unused
}
