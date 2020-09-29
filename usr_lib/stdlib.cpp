#include <stdlib.h>
#include <klib/mem_util.h>
#include <ctype.h>
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
