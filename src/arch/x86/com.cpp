#include <arch/x86/com.hpp>
#include <arch/x86/port.hpp>

#include "libcore/enum-op.hpp"
#include "libcore/result.hpp"
#include "libcore/str.hpp"
#include "libcore/type-utils.hpp"

namespace arch::x86
{

void Com::write_reg(Com::Register reg, uint8_t value)
{
    arch::x86::out8(static_cast<uint16_t>(_port) + static_cast<uint16_t>(reg), value);
}

uint8_t Com::read_reg(Com::Register reg) const
{
    return arch::x86::in8(static_cast<uint16_t>(_port) + static_cast<uint16_t>(reg));
}

void Com::update_baud_rate(int baud_rate)
{
    uint16_t divisor = 115200 / baud_rate;
    write_reg(Com::Register::LINE_CONTROL, core::underlying_value(Com::LineControl::DLAB_STATUS));
    write_reg(Com::Register::BAUD_RATE_LOW, divisor & 0xFF);
    write_reg(Com::Register::BAUD_RATE_HIGH, (divisor >> 8) & 0xFF);
    write_reg(Com::Register::LINE_CONTROL, core::underlying_value(Com::LineControl::DATA_SIZE_8));
}

bool Com::can_write() const
{
    return read_reg(Com::Register::LINE_STATUS) & core::underlying_value(Com::LineStatus::TRANSMITTER_BUFFER_EMPTY);
}

void Com::putc(char value)
{
    while (!can_write())
    {
        asm volatile("pause");
    }
    write_reg(Com::Register::DATA, value);
}

core::Result<void> Com::write(const char *str, size_t size)
{
    _lock.lock();
    for (size_t i = 0; i < size; i++)
    {
        putc(str[i]);
    }
    _lock.release();
    return {};
}

core::Result<Com> Com::initialize(Com::Port port)
{

    Com com = {};
    com._port = port;

    com.write_reg(Com::Register::INTERRUPT_IDENTIFICATION, 0);

    com.update_baud_rate(115200); // max

    com.write_reg(Com::Register::MODEM_CONTROL, core::underlying_value(Com::Modem::DTR | Com::Modem::RTS | Com::Modem::OUT2 | Com::Modem::OUT1));

    com.write_reg(Com::Register::INTERRUPT_ENABLE, core::underlying_value(Com::InterruptEnable::WHEN_DATA_AVAILABLE));

    return (com);
}

} // namespace arch::x86
