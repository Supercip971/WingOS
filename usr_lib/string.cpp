#include <stdint.h>
uint64_t strlen(const char* s){
    uint64_t string_length = 0;
    while(s[string_length] != 0){
        string_length++;
    }
    return string_length;
}
