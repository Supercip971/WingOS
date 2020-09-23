#include <stdint.h>
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
#ifndef WOS_OPTIMIZATION
void* memcpy(void* dest, const void* src, uint64_t length){
    char* cdest = (char*) dest;
    const char* csrc = (const char*) src;
    for(uint64_t i = 0; i < length  ; i++ ){
        cdest[i] = csrc[i];
    }
    return cdest;
}
// for later optimization
#else





#endif
