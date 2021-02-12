#pragma once
#include <stddef.h>
#include <stdint.h>
enum xsave_reg
{
    x87_XSAVE_REG = (1 << 0),
    SSE_XSAVE_REG = (1 << 1),
    AVX_XSAVE_REG = (1 << 2),

};
#define SSE_LOW_LEVEL_FUNC __attribute__((optimize("O0")))

SSE_LOW_LEVEL_FUNC bool has_xsave();
SSE_LOW_LEVEL_FUNC bool has_avx();

uint32_t get_xsave_size();
void init_sse();

void save_sse_context(uint64_t *context);

void load_sse_context(uint64_t *context);
