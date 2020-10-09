#pragma once
#include <stddef.h>
#include <stdint.h>
int abs(int j);

double strtod(const char* nptr, char** endptr);
long long strtoll(const char* nptr, char** endptr, int base);
double atof(const char* s);

#define RAND_MAX 32767
int rand(void);
void srand(uint32_t seed);
