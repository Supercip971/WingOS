#include <arch/x86_64/gdt.hpp>

namespace arch::amd64
{

static Gdtr _default_gdtr;
static Gdt _default_gdt;
Gdtr* default_gdt()
{
    _default_gdt = Gdt();
    _default_gdtr = Gdtr{
        reinterpret_cast<uintptr_t>(&_default_gdt),
        sizeof(_default_gdt) - 1,
    };
    return &_default_gdtr;
    
}

extern "C" void gdtr_install(Gdtr* gdtr);
void load_gdt(Gdtr* gdtr)
{
    gdtr_install(gdtr);
}

} // namespace arch::amd64