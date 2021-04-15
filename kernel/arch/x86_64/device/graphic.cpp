#include "graphic.h"
#include <logging.h>
#include <physical.h>
#include <virtual.h>
basic_framebuffer_graphic_device::basic_framebuffer_graphic_device(size_t width, size_t height, uintptr_t physical_addr, framebuff_bpp bpp)
{
    if (bpp == BPP_32_BIT)
    {

        log("framebuffer_graphic_device", LOG_INFO, "loading basic framebuffer device, with width: {} height: {} at addr {}", width, height, physical_addr);
        _width = width;
        _height = height;

        _addr = reinterpret_cast<void *>(get_mem_addr(physical_addr));
    }
    else
    {
        log("framebuffer_graphic_device", LOG_WARNING, "basic_framebuffer_graphic_device don't support 24bit pixel for the moment");
        // unsupported rn
    }
}
