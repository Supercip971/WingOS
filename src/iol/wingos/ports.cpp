#include "iol/ports.hpp"

#include "iol/wingos/syscalls.h"
#include "wingos-headers/syscalls.h"
namespace iol
{

void outb(uint16_t port, uint8_t value)
{
    sys$ipc_x86_port_out(port, 1, value);
}

uint8_t inb(uint16_t port)
{
    return sys$ipc_x86_port_in(port, 1);
}

void outw(uint16_t port, uint16_t value)
{
    sys$ipc_x86_port_out(port, 2, value);
}

uint16_t inw(uint16_t port)
{
    return sys$ipc_x86_port_in(port, 2);
}

void outl(uint16_t port, uint32_t value)
{
    sys$ipc_x86_port_out(port, 4, value);
}

uint32_t inl(uint16_t port)
{
    return sys$ipc_x86_port_in(port, 4);
}
} // namespace iol