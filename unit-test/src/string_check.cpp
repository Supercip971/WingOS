#include "string_check.h"
#include <string.h>


int strlen_check_0(){ 
    size_t str_length = 5;
    const char* str = "hello";
    size_t str2_length =30;
    const char* str2 = "012345679012345678901234567890";
    if(strlen(str) != str_length){
        return 1;
    }if(strlen(str2) != str2_length){
        return 2;
    }
    return 0;
}
int strnlen_check_0(){ 
    size_t str_length = 5;
    const char* str = "hello";
    size_t str2_length =30;
    const char* str2 = "012345679012345678901234567890";
    if(strnlen(str, 100) != str_length){
        return 1;
    }if(strnlen(str2, 100) != str2_length){
        return 2;
    }
    return 0;
}
int strnlen_check_1(){
    const char* str2 = "012345679012345678901234567890";
    if(strnlen(str2, 3) > 3){
        return 1;
    }
    if(strnlen(str2, 7) > 7){

        return 2;
    }
    if(strnlen(str2, 30) > 30){
        return 3;
    }
    return 0;
}

int strcmp_check_true(){
    const char* a1 = "hello";
    const char* a2 = "hello";

    const char* b1 = "a";
    const char* b2 = "a";

    const char* c1 = "gnu/linux";
    const char* c2 = "gnu/linux";

    if(strcmp(a1,a2) != 0){
        return -1;
    }
    if(strcmp(b1,b2) != 0){
        return -2;
    }
    if(strcmp(c1,c2) != 0){
        return -3;
    }

    return 0;
}
int strcmp_check_false(){

    const char* a1 = "hello";
    const char* a2 = "heLlO";

    const char* b1 = "testing";
    const char* b2 = "testingtest";

    const char* c1 = "c++ powaaaa";
    const char* c2 = "ansi c powaaaa";

    if(strcmp(a1,a2) == 0){
        return -1;
    }
    if(strcmp(b1,b2) == 0){
        return -2;
    }
    if(strcmp(c1,c2) == 0){
        return -3;
    }
    return 0;
}