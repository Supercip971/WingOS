#include <stdlib.h>
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

