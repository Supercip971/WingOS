#pragma once 


#include <stddef.h>

// malloc 
#ifdef __cplusplus
extern "C" {
#endif

int abs(int x);
long labs(long j);
long long llabs(long long x);

double fabs(double x);
float fabsf(float x);
long double fabsl(long double x);


void *malloc(size_t size);
void free(void *ptr);
void *realloc(void *ptr, size_t size);

[[noreturn]]
void exit(int code);

long strtol(const char *nptr, char **endptr, int base);
long long strtoll(const char *nptr, char **endptr, int base);
int atoi(const char* str);

double strtod(const char* nptr, char** endptr);

double atof(const char* str);
#ifdef __cplusplus
}
#endif



