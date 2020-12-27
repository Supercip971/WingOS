#pragma once
#include <stdint.h>

void init_sse();

void save_sse_context(uint64_t *context);

void load_sse_context(uint64_t *context);
