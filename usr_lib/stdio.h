#pragma once
#include <stdarg.h>


int printf(const char* format, ...);


int vsprintf(char* buffer, const char* format, va_list vlist);
int sprintf(char* buffer, const char* format, ...);
