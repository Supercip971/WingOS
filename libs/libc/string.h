#ifndef STRING_H
#define STRING_H
#include <stddef.h>
#include <stdint.h>
size_t strlen(const char *s);
size_t strnlen(const char *s, size_t maxlen);

int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t n);
char *strcpy(char *dest, const char *src);
char *strncpy(char *dest, const char *src, size_t n);
char *strtok(char *s, const char delimeter);

int memcmp(const void *s1, const void *s2, size_t n);
void *memcpy(void *dest, const void *src, size_t length);
void *memmove(void *dest, const void *src, size_t n);
void *memset(void *data, int value, size_t lenght);
void *memchr(const void *s, int c, size_t n);

#endif