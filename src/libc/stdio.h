#pragma once 


#ifdef __cplusplus
extern "C" {
    #endif
#include <stddef.h>

typedef struct FILE FILE;

extern FILE* stdin;
extern FILE* stdout;
extern FILE* stderr;

size_t fwrite(void* __restrict ptr, size_t size, size_t n, FILE* __restrict file );
size_t fread(void* __restrict ptr, size_t size, size_t n, FILE* __restrict file );

int fprintf(FILE *__restrict stream, const char *__restrict format, ...);
int printf( const char *__restrict format, ...);


int snprintf ( char * s, size_t n, const char * format, ... );

#ifdef __cplusplus 
}
#endif