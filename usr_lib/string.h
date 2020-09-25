#pragma once
#include <stdint.h>

uint64_t strlen(const char* s);
int strcmp(const char* s1, const char* s2);
int strncmp(const char *s1, const char *s2, uint64_t n);
char* strcpy(char* dest, const char* src);
char* strncpy(char* dest, const char* src, uint64_t n);



void* memcpy(void* dest, const void* src, uint64_t length);

