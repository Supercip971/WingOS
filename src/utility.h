#pragma once

#include <arch/mem/memory_manager.h>
#include <int_value.h>
#include <logging.h>
#include <stddef.h>
int isdigit(int c);
char *strtok(char *s, const char *delm);

int64_t strtoint(const char *nptr);

int strncmp(const char *s1, const char *s2, size_t n);
int strcmp(const char *s1, const char *s2);
size_t strlen(const char *s);

void kitoa(char *buf, int base, int d);
void kitoa64(char *buf, int base, int64_t d);
void kitoaT(char *buf, int base, size_t d);
