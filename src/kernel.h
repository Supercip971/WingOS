#pragma once
#include <int_value.h>
#include <stivale.h>
#include <com.h>
void _start(struct stivale_struct *bootloader_data) ;

__attribute__((optimize("O0"))) inline void  memzero(void * s, uint64_t n) {
    for (uint64_t i = 0; i < n; i++)
    {
        ((uint8_t*)s)[i] = 0;
    }
}
