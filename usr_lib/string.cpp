#include <stdint.h>
#include <klib/kernel_util.h>
#include <string.h>
#include <klib/mem_util.h>
uint64_t strlen(const char* s){
    uint64_t string_length = 0;
    while(s[string_length] != 0){
        string_length++;
    }
    return string_length;
}
int strcmp(const char* s1, const char* s2){

    while(*s1 == *s2 && (*s1)){
        s1++;
        s2++;
    }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}
int strncmp(const char *s1, const char *s2, uint64_t n)
{
    while (n && *s1 && (*s1 == *s2))
    {
        ++s1;
        ++s2;
        --n;
    }
    if (n == 0)
    {
        return 0;
    }
    else
    {
        return (*(unsigned char *)s1 - *(unsigned char *)s2);
    }
}

char* strcpy(char* dest, const char* src){
    uint64_t i = 0;
    while(src[i] != '\0'){
        dest[i] = src[i];
        i++;
    }
    return dest;
}
char* strncpy(char* dest, const char* src, uint64_t n)  {
    uint64_t i;
    for (i = 0; i < n && src[i] != '\0'; i++){
        dest[i] = src[i];
    }
    for ( ; i < n; i++){
        dest[i] = '\0';
    }

    return dest;
}

#ifndef WOS_OPTIMIZATION
void* memcpy(void* dest, const void* src, uint64_t length){
    char* cdest = (char*) dest;
    const char* csrc = (const char*) src;
    for(uint64_t i = 0; i < length  ; i++ ){
        cdest[i] = csrc[i];
    }
    return cdest;
}

int memcmp(const void* s1, const void* s2, size_t n){
    const char* ss1 = (const char*) s1;
    const char* ss2 = (const char*) s2;
    for(size_t i = 0; i < n; i++){
        if(ss1[i] != ss2[i]){
            return false;
        }
    }
    return true;
}

void* memmove(void* dest, const void* src, size_t n){
    char* new_dst = (char*)dest;
    const char* new_src = (const char*)src;

    char* temporary_data = (char*)sys::service_malloc(n);


    for(size_t i = 0; i < n; i++){
        temporary_data[i] = new_src[i];
    }
    for(size_t i = 0; i < n; i++){
        new_dst[i] = temporary_data[i];
    }



    sys::service_free(temporary_data);
    return dest;
}
// for later optimization
#else





#endif
