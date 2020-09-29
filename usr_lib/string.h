#pragma once
#include <stdint.h>
#include <stddef.h>
uint64_t strlen(const char* s);
int strcmp(const char* s1, const char* s2);
int strncmp(const char *s1, const char *s2, uint64_t n);
char* strcpy(char* dest, const char* src);
char* strncpy(char* dest, const char* src, uint64_t n);



int memcmp(const void* s1, const void* s2, size_t n);
void* memcpy(void* dest, const void* src, uint64_t length);
void* memmove(void* dest, const void* src, size_t n);
