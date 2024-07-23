#pragma once

#include <arch/x86/port.hpp>

namespace hw
{
class Pic
{

public:
    enum PicRegisters : uint8_t
    {
        PIC1_COMMAND = 0x20,
        PIC1_DATA = 0x21,
        PIC2_COMMAND = 0xa0,
        PIC2_DATA = 0xa1,
    };
    static void disable()
    {

        arch::x86::out8(PicRegisters::PIC1_DATA, 0xff);
        arch::x86::out8(PicRegisters::PIC2_DATA, 0xff);
    }
};
} // namespace hw