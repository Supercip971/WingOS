#include "string_check.h"
#include <string.h>
#include <stdint.h>
#include <stdio.h>
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


int memcmp_check_true(){
    uint8_t a1[] = { 0, 0, 0, 10, 10, 20};
    uint8_t a2[] = { 0, 0, 0, 10, 10, 20};

    uint8_t b1[] = { 1};
    uint8_t b2[] = { 1};

    uint8_t c1[] = { 0,5,0,0,10,10};
    uint8_t c2[] = { 0,5,0,0,10,10};

    if(!memcmp(a1,a2, sizeof(a1))){
        return -1;
    }if(!memcmp(b1,b2, sizeof(b1))){
        return -2;
    }if(!memcmp(c1,c2, sizeof(c1))){
        return -3;
    }
    return 0;
}
int memcmp_check_false(){
    uint8_t a1[] = { 0, 0, 0, 10, 10, 20};
    uint8_t a2[] = { 0, 0, 0, 10, 10, 30};

    uint8_t b1[] = { 0};
    uint8_t b2[] = { 1};

    uint8_t c1[] = { 1,0,0,0,10,10};
    uint8_t c2[] = { 0,0,0,0,10,10};

    if(memcmp(a1,a2, sizeof(a1))){
        return -1;
    }if(memcmp(b1,b2, sizeof(b1))){
        return -2;
    }if(memcmp(c1,c2, sizeof(c1))){
        return -3;
    }
    return 0;
}

int memset_check_0(){ 
    uint8_t buffer[512];
    for(int i = 16; i < 254; i++){

        for(int j = 0; j < 512; j++){
            buffer[j] = 0;
        }
        memset(buffer, i, i);
        for(int j = 0; j < 512; j++){
            if(j < i){
                if(buffer[j] != i){
                    printf("bad memset %i %i should be i \n", i, j);
                    return i << 16 | j;
                }
            }else{
                if(buffer[j] != 0){
                    printf("bad memset %i %i should be 0 \n", i, j);
                    return i << 16 | j;
                }
            }
        }
    }
    return 0;
}