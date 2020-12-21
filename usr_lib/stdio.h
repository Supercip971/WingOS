#ifndef STDIO_H
#define STDIO_H
#include <stdarg.h>
#include <stdint.h>
int printf(const char *format, ...);
int vsprintf(char *buffer, const char *format, va_list vlist);
int sprintf(char *buffer, const char *format, ...);
#endif