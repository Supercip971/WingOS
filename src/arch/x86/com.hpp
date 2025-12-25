#pragma once

#include <arch/x86/port.hpp>
#include <libcore/enum-op.hpp>
#include <libcore/io/writer.hpp>
#include <libcore/result.hpp>
#include <libcore/lock/lock.hpp>
#include <stdint.h>
#include "libcore/type-utils.hpp"
namespace arch::x86
{

class Com : public core::Writer, public core::NoCopy
{
    core::Lock _lock = {};
public:
    Com() = default;
    Com(Com &&) = default;
    Com &operator=(Com &&) = default;
    enum class Port : uint16_t
    {
        COM1 = 0x3F8,
        COM2 = 0x2F8,
        COM3 = 0x3E8,
        COM4 = 0x2E8
    };

    Port port() const
    {
        return _port;
    }
    enum class Register : uint16_t
    {
        DATA = 0,
        INTERRUPT_ENABLE = 1,
        BAUD_RATE_LOW = 0,
        BAUD_RATE_HIGH = 1,
        INTERRUPT_IDENTIFICATION = 2,
        FIFO_CONTROLLER = 2,
        LINE_CONTROL = 3,
        MODEM_CONTROL = 4,
        LINE_STATUS = 5,
        MODEM_STATUS = 6,
        SCRATCH = 7
    };

    enum class LineControl : uint8_t
    {

        DATA_SIZE_5 = 0,
        DATA_SIZE_6 = 1,
        DATA_SIZE_7 = 2,
        DATA_SIZE_8 = 3,
        DLAB_STATUS = 1 << 7,
    };

    enum class Modem : uint8_t
    {
        DTR = 1 << 0,
        RTS = 1 << 1,
        OUT1 = 1 << 2,
        OUT2 = 1 << 3,
        LOOPBACK = 1 << 4
    };

    enum class InterruptEnable : uint8_t
    {
        WHEN_DATA_AVAILABLE = 1 << 0,
        WHEN_TRANSMITTER_EMPTY = 1 << 1,
        WHEN_ERROR = 1 << 2,
        WHEN_MODEM_STATUS_UPDATE = 1 << 3
    };

    enum class LineStatus : uint8_t
    {
        DATA_READY = 1 << 0,
        OVERRUN_ERROR = 1 << 1,
        PARITY_ERROR = 1 << 2,
        FRAMING_ERROR = 1 << 3,
        BREAK_INDICATOR = 1 << 4,
        TRANSMITTER_BUFFER_EMPTY = 1 << 5,
        TRANSMITTER_EMPTY = 1 << 6,
        ERROR_IN_FIFO = 1 << 7
    };

    void write_reg(Com::Register reg, uint8_t value);

    uint8_t read_reg(Com::Register reg) const;

    void update_baud_rate(int baud_rate);

    void putc(char c);

    bool can_write() const;

    void wait_write();

    core::Result<void> write(const char *data, size_t size) override;

    static core::Result<Com> initialize(Com::Port port);
    template <core::Viewable T>
    constexpr core::Result<void> write(T view)
        requires(core::Viewable<T>)
    {
        _lock.lock();
        auto res = write(view.data(), view.len());
        _lock.release();
        return res;
    }

    ~Com()
    {
    }

private:
    Port _port{};
};

ENUM_OP$(Com::LineControl);
ENUM_OP$(Com::Modem);
ENUM_OP$(Com::InterruptEnable);
ENUM_OP$(Com::LineStatus);

} // namespace arch::x86