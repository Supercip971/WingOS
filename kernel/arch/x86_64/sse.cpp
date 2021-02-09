#include <utils/lock.h>
#define SSE_LOW_LEVEL_FUNC __attribute__((optimize("O2")))
#include <device/local_data.h>
#include <logging.h>
#include <sse.h>
ASM_FUNCTION void sse_init(void);
ASM_FUNCTION void avx_init(void);

ASM_FUNCTION void asm_sse_save(uintptr_t addr);
ASM_FUNCTION void asm_sse_load(uintptr_t addr);

uint64_t *fpu_reg;
uint64_t fpu_data[128] __attribute__((aligned(16)));
utils::lock_type sse_lock;

SSE_LOW_LEVEL_FUNC void init_sse()
{

    fpu_reg = (uint64_t *)fpu_data;
    sse_init();
    asm volatile("fninit");

    asm_sse_save(((uint64_t)fpu_data));
}

SSE_LOW_LEVEL_FUNC void save_sse_context(uint64_t *context)
{
    sse_lock.lock();
    asm_sse_save(((uintptr_t)fpu_data));
    for (int i = 0; i < 128; i++)
    {
        context[i] = fpu_data[i];
    }
    sse_lock.unlock();
}

SSE_LOW_LEVEL_FUNC void load_sse_context(uint64_t *context)
{

    sse_lock.lock();
    for (int i = 0; i < 128; i++)
    {
        fpu_data[i] = context[i];
    }
    asm_sse_load(((uintptr_t)fpu_data));

    sse_lock.unlock();
}
