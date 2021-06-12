#include <device/local_data.h>
#include <logging.h>
#include <sse.h>
#include <utils/lock.h>
#include <utils/sys/config.h>

ASM_FUNCTION void sse_init(void);
ASM_FUNCTION void avx_init(void);

ASM_FUNCTION void enable_xsave(void);

ASM_FUNCTION void asm_sse_save(uintptr_t addr);
ASM_FUNCTION void asm_sse_load(uintptr_t addr);

ASM_FUNCTION void asm_avx_save(uintptr_t addr);
ASM_FUNCTION void asm_avx_load(uintptr_t addr);

bool use_xsave = false;
bool checked_xsave = false;

uint64_t *fpu_reg = nullptr;

size_t xsave_size;

uint64_t fpu_data[128] __attribute__((aligned(64)));

utils::lock_type sse_lock;

SSE_LOW_LEVEL_FUNC bool has_xsave()
{

    uint32_t ecx = 0;
    asm volatile("cpuid"
                 : "=c"(ecx)
                 : "a"(1), "c"(0));
    return ecx & (((uint32_t)1) << 26);
}

SSE_LOW_LEVEL_FUNC bool has_avx()
{
    uint32_t ecx = 0;
    asm volatile("cpuid"
                 : "=c"(ecx)
                 : "a"(1), "c"(0));
    return ecx & (1 << 28);
}

uint32_t get_sse_size()
{

    if (has_xsave())
    {

        uint32_t ecx = 0;
        asm volatile("cpuid"
                     : "=c"(ecx)
                     : "a"(0xd), "c"(0));
        return ecx;
    }
    else
    {
        return 512;
    }
}

SSE_LOW_LEVEL_FUNC void init_xsave()
{

#ifdef USE_AVX
    if (has_xsave())
    {

        enable_xsave();

        if (has_avx())
        {
            avx_init();
        }

        xsave_size = get_sse_size();
        use_xsave = true;
    }
#else
    use_xsave = false;
#endif
}

SSE_LOW_LEVEL_FUNC void init_sse()
{
    sse_lock.lock();

    sse_init();

#ifdef USE_AVX
    init_xsave();
#endif
    asm("fninit");
    sse_lock.unlock();
    asm_avx_save((uintptr_t)(fpu_data));
}

SSE_LOW_LEVEL_FUNC void save_sse_context(uint8_t *context)
{
    if (use_xsave)
    {
        asm_avx_save((uintptr_t)(context));
    }
    else
    {
        asm_sse_save((uintptr_t)(context));
    }
}

SSE_LOW_LEVEL_FUNC void load_sse_context(uint8_t *context)
{

    if (use_xsave)
    {
        asm_avx_load((uintptr_t)context);
    }
    else
    {
        asm_sse_load(((uintptr_t)context));
    }
}
