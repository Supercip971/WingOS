#include <stdint.h>
uint64_t strlen(const char* s){
    uint64_t string_length = 0;
    while(s[string_length] != 0){
        string_length++;
    }
    return string_length;
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
