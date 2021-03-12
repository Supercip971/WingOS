#include "alloc_array_check.h"
#include <utils/alloc_array.h>


int alloc_array_creation_test(){
    utils::alloc_array<int, 64> array;
    if(array.size() != 64){
        return -1;
    }

    if(array.allocated_element_count() != 0){
        return -2;
    }

    for(int i= 0; i < 64;i++){
        if(array.status(i) != false){
            return i+1024;
        }
    }
    return 0;
}

int alloc_array_creation_fill_test() {
    utils::alloc_array<int, 64> array(0);
    if(array.size() != 64){
        return -1;
    }

    if(array.allocated_element_count() != 0){
        return -2;
    }

    for(int i= 0; i < 64;i++){
        if(array.status(i) != false){
            return i+1024;
        }
    }
    return 0;  
}

int alloc_array_alloc_test(){

    utils::alloc_array<int, 1024> array;
    for(int i = 0; i < 512; i++){
        size_t targ = array.alloc();
        if(!array.status(targ)){
            return i + 2048;
        }
        array[targ] = targ;
    }
    if(array.allocated_element_count() != 512){
        return -2;
    }

    size_t detected_allocated_element = 0;

    for(int i = 0; i < 1024; i++){
        if(array.status(i)){
            detected_allocated_element++;
            if(array[i] != i){
                return i + 1024;
            }
        }
    }
    if(detected_allocated_element != 512){
        return -1;
    }
    return 0;

}
int alloc_array_free_test(){
    utils::alloc_array<int, 1024> array;
    for(int i = 0; i < 1024; i++){
        size_t targ = array.alloc();
        if(!array.status(targ)){
            return i + 2048;
        }
        if((i % 2) == 0){
            if(!array.free(targ)){
                return i + 4096;
            }
        }else{

            array[targ] = targ;
        }
    }
    if(array.allocated_element_count() != 512){
        return -2;
    }

    size_t freed_element_count = 0;

    for(int i = 0; i < 512; i++){
        if(array.status(i)){
            freed_element_count++;

            if(array[i] != i){
                return i + 1024;
            }
            if(!array.free(i)){
                return i + 8192;
            }
        }
    }
    if(freed_element_count != 512){
        return -1;
    }
    return 0;
}
int alloc_array_foreachentry_test(){
    utils::alloc_array<int, 1024> array;
    for(int i = 0; i < 512; i++){
        array.alloc();
    }
    size_t detected_entry_count = 0;
    array.foreach_entry([&](int& value){
        detected_entry_count++;
    });
    if(detected_entry_count != 512){
        return detected_entry_count + 1024;
    }
    return 0;

}
