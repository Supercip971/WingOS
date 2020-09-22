#pragma once
#include <stdint.h>

uint64_t strlen(const char* s);

int strcmp(const char* s1, const char* s2);
void* memcpy(void* dest, const void* src, uint64_t length);
