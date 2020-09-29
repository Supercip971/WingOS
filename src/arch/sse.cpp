#include <arch/mem/liballoc.h>
#include <arch/sse.h>
#include <loggging.h>
extern "C" void sse_init(void);
uint64_t *fpu_reg;
uint64_t fpu_data[128];
extern "C" void asm_sse_save(uint64_t addr);
extern "C" void asm_sse_load(uint64_t addr);
void init_sse()
{
    // uint64_t fpu_data = (uint64_t)malloc(128 * sizeof(uint64_t));

    fpu_reg = (uint64_t *)fpu_data;
    sse_init();
    asm volatile("fninit");

    //  asm volatile("fxsave (%0)" ::"r"((uint64_t)fpu_reg));
    asm_sse_save((uint64_t)fpu_reg);
}

void save_sse_context(uint64_t *context)
{
    //asm volatile("fxsave (%0)" ::"r"((uint64_t)fpu_reg));
    asm_sse_save((uint64_t)fpu_reg);
    for (int i = 0; i < 128; i++)
    {
        context[i] = fpu_reg[i];
    }
}
void load_sse_context(uint64_t *context)
{
    for (int i = 0; i < 128; i++)
    {
        fpu_reg[i] = context[i];
    }
    asm_sse_load((uint64_t)fpu_reg);

    //asm volatile("fxrstor (%0)" ::"r"((uint64_t)fpu_reg));
}
