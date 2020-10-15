#pragma once
#include <stddef.h>
#include <stdint.h>
size_t strlen(const char *s);
int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t n);
char *strcpy(char *dest, const char *src);
char *strncpy(char *dest, const char *src, size_t n);

int memcmp(const void *s1, const void *s2, size_t n);
void *memcpy(void *dest, const void *src, size_t length);
void *memmove(void *dest, const void *src, size_t n);
void *memset(void *data, uint8_t value, size_t lenght);
void *memchr(const void *s, int c, size_t n);
