
#include <libcore/fmt/log.hpp>
#include <mcx/mcx.hpp>

#include "limine.hpp"

__attribute__((used)) static volatile struct limine_memmap_request mmap_request = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0,
};

__attribute__((used)) static volatile struct limine_framebuffer_request limine_framebuffer_req = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0};
__attribute__((used)) static volatile struct limine_rsdp_request rsdp_request = {
    .id = LIMINE_RSDP_REQUEST,
    .revision = 0};
static mcx::MemoryMap::Type limine_type_to_mcx(int type)
{
    switch (type)
    {
    case LIMINE_MEMMAP_USABLE:
        return mcx::MemoryMap::Type::FREE;
    case LIMINE_MEMMAP_RESERVED:
        return mcx::MemoryMap::Type::RESERVED;
    case LIMINE_MEMMAP_ACPI_RECLAIMABLE:
        return mcx::MemoryMap::Type::ACPI_RECLAIMABLE;
    case LIMINE_MEMMAP_ACPI_NVS:
        return mcx::MemoryMap::Type::ACPI_NVS;
    case LIMINE_MEMMAP_BAD_MEMORY:
        return mcx::MemoryMap::Type::BAD_MEMORY;
    case LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE:
        return mcx::MemoryMap::Type::BOOTLOADER_RECLAIMABLE;
    case LIMINE_MEMMAP_KERNEL_AND_MODULES:
        return mcx::MemoryMap::Type::KERNEL_AND_MODULES;
    case LIMINE_MEMMAP_FRAMEBUFFER:
        return mcx::MemoryMap::Type::FRAMEBUFFER;
    default:
        return mcx::MemoryMap::Type::RESERVED;
    }
}

static void load_mcx_mmap(mcx::MachineContext *context)
{
    struct limine_memmap_entry **mmap = mmap_request.response->entries;
    size_t mmap_size = mmap_request.response->entry_count;

    for (size_t i = 0; i < mmap_size; i++)
    {

        context->_memory_map[i].range = mcx::MemoryRange::from_begin_len(mmap[i]->base, mmap[i]->length);
        context->_memory_map[i].type = limine_type_to_mcx(mmap[i]->type);
    }
    context->_memory_map_count = mmap_size;

    context->_framebuffer = mcx::MachineFramebuffer{
        .address = limine_framebuffer_req.response->framebuffers[0]->address,
        .width = (int)limine_framebuffer_req.response->framebuffers[0]->width,
        .height = (int)limine_framebuffer_req.response->framebuffers[0]->height,
        .pitch = (int)limine_framebuffer_req.response->framebuffers[0]->pitch,

        .bpp = limine_framebuffer_req.response->framebuffers[0]->bpp,
    };

    context->_rsdp = (uintptr_t)rsdp_request.response->address;
}
void load_mcx(mcx::MachineContext *context)
{
    load_mcx_mmap(context);
}