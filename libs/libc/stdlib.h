#ifndef STDLIB_H
#define STDLIB_H
#include <stddef.h>
#include <stdint.h>
int abs(int j);
long labs(long j);
long long llabs(long long j);

#ifdef __SSE__
double strtod(const char *nptr, char **endptr);
#endif

long long strtoll(const char *nptr, char **endptr, int base);
long strtol(const char *nptr, char **endptr, int base);

#ifdef __SSE__
double atof(const char *s);
#endif
long atoi(const char *s);

#define RAND_MAX 32767
int rand(void);
void srand(uint32_t seed);

void *malloc(size_t size);
void free(void *ptr);
void *realloc(void *ptr, size_t size);

void exit(int status);
#endif
