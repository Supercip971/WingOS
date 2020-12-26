#ifndef STDIO_H
#define STDIO_H
#include <klib/file.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
int printf(const char *format, ...);
int vsprintf(char *buffer, const char *format, va_list vlist);
int sprintf(char *buffer, const char *format, ...);

struct FILE
{
    sys::file file_element;
};

#define SEEK_SET 2
#define SEEK_CUR 3
#define EOF 0x0E0F
FILE *fopen(const char *pathname, const char *mode);
int fclose(FILE *stream);

size_t fread(void *ptr, size_t size, size_t count, FILE *stream);
int fgetc(FILE *stream);

int ungetc(int c, FILE *stream); // not implemented

int fseek(FILE *stream, long offset, int whence);
long ftell(FILE *stream);

int feof(FILE *stream);
int ferror(FILE *stream);
#endif
