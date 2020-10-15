#include <stdlib.h>
#include <klib/mem_util.h>
#include <klib/string_util.h>
#include <stddef.h>
#include <ctypes.h>
#include <stdio.h>
int abs(int j){
    if(j < 0){
        return -j;
    }
    return j;
}

double atof(const char* s){
    double result = 0.0;
    int offset = 0;
    int temp_char = 0;
    while(isdigit(temp_char = *s++)){
        result = result*10 + (temp_char- '0');
    }
    if(temp_char == '.'){
        while(isdigit(temp_char = *s++)){
            result = result*10 + (temp_char- '0');
        }
        offset = offset - 1;

    }else{
        return result;
    }


    while(offset < 0){
        result *= 0.1;
        offset += 1;
    }
    return result;
}

double strtod(const char* nptr, char** endptr){
    double current_num;
    const char* s = nptr;
    const char* end;
    char* end_buffer;
    int current_error;
    while(*s == ' '){
        s++;
    }
    bool is_neg = false;
    if(*s == '-'){
        is_neg = true;
        s++;
    }

    const char* d2 = s + 1;
    double res = atof(d2);
    if(is_neg == true){
        res *= -1;
    }
    if(endptr != 0){
        *endptr = (char*)d2;
    }
    return res;
}

long long strtoll(const char* nptr, char** endptr, int base){

    const char* s = nptr;
    const char* end;
    long long ret_value = 0;
    char* end_buffer;
    int current_error;
    bool is_neg = false;

    while(*s == ' '){
        s++;
    }
    if(*s == '-'){
        is_neg = true;
        s++;
    }
    // if base == 0 and detect 0x at the start of string put it at 16
    if((base == 0 || base == 16) && s[0] == '0' && (s[1] == 'x' || s[1] == 'X')){
        s += 2;
        base = 16;
    }
    if(base == 0){
        base = 10;

    }

    char* start = (char*)s;
    while(isalnum(*s)){
        if(base == 10){
            if(isdigit(*s)){
                ret_value = (*s - '0')*base;
            }else{
                break;
            }
        }else if(base == 16){
            if(isdigit(*s)){

                ret_value = (*s - '0')*base;
            }else{
                ret_value = ((to_lower(*s) - 'a')+10)*base;
            }
        }
        s++;
    }
    if(is_neg){
        ret_value *= -1;
    }
    if(endptr != 0){
        *endptr = start;
    }
    return ret_value;
}

extern "C" void __cxa_pure_virtual(){
    printf("error __cxa_pure_virtual() called");
}

unsigned long int cseed = 1;

int rand(){
    cseed = cseed * 1103515245 + 12345;
    return (unsigned int)(cseed/65536) % 32768;
}

void srand(uint32_t seed){
    cseed = seed;
}

void* malloc(size_t size){
    return sys::service_malloc(size);
}

void free(void* ptr){
    sys::service_free(ptr);
}

void* realloc(void* ptr, size_t size){
    return sys::service_realloc(ptr, size);
}
