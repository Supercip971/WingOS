#pragma once 


#ifdef __cplusplus
extern "C" {
    #endif
#include <stddef.h>
#include <stdarg.h>

typedef struct FILE FILE;

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2
extern FILE* stdin;
extern FILE* stdout;
extern FILE* stderr;

size_t fwrite(void* __restrict ptr, size_t size, size_t n, FILE* __restrict file );
size_t fread(void* __restrict ptr, size_t size, size_t n, FILE* __restrict file );

// remove and rename
int remove(const char* filename);
int rename(const char* old_filename, const char* new_filename);

int mkdir(const char* pathname, unsigned int mode);


void fflush(FILE* stream);

int puts(const char* str);

int sscanf(const char* __restrict str, const char* __restrict format, ...);

int putchar(int c);
int fprintf(FILE *__restrict stream, const char *__restrict format, ...);
int printf( const char *__restrict format, ...);

int vfprintf(FILE *__restrict stream, const char *__restrict format, va_list arg);
int vsnprintf(char * __restrict s, size_t n, const char * __restrict format, va_list arg);


FILE* fopen(const char* filename, const char* mode);
int fclose(FILE* stream);
int fseek(FILE* stream, long offset, int origin);
long ftell(FILE* stream);


int snprintf ( char * s, size_t n, const char * format, ... );

#ifdef __cplusplus 
}
#endif