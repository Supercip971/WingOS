#pragma once
#include <device/general_device.h>

enum framebuff_bpp
{
    BPP_24_BIT = 2,
    BPP_32_BIT = 3,
};

class basic_framebuffer_graphic_device : public generic_framebuffer
{
protected:
    size_t _width;
    size_t _height;
    void *_addr;

public:
    basic_framebuffer_graphic_device(size_t width, size_t height, uintptr_t phys_addr, framebuff_bpp bpp = framebuff_bpp::BPP_32_BIT);
    virtual uintptr_t get_addr() { return reinterpret_cast<uintptr_t>(_addr); };
    virtual size_t width() { return _width; };
    virtual size_t height() { return _height; };
};
