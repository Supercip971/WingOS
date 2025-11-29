#ifndef STRING_H
#define STRING_H
#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

    void *memcpy(void *__restrict dest, const void *__restrict src, size_t n);
    void *memset(void *s, int c, size_t n);

    char *strchr(const char *s, int c);
    char *strrchr(const char *s, int c);

    char* strncpy( char * __restrict dest, const char * __restrict src, size_t n);

    char* strdup(const char* s);
    int strcmp(const char* s1, const char* s2);

    size_t strlen(const char *s);

    char *strerror(int errnum);

    char *strtok(char * __restrict str, const char * __restrict delim);
    char *strtok_r(char * __restrict str, const char * __restrict delim,
                   char **__restrict saveptr);


    char *strpbrk(const char *s, const char *accept);


    void * memmove(void* dest, const void* str, size_t n);



#ifdef __cplusplus
}
#endif

#endif