#include "math_check.h"
#include <utils/math.h>
int min_check_utils(){ 
    if(utils::min(10,9) != 9){
        return -1;
    }if(utils::min(9,10) != 9){
        return -2;
    }if(utils::min(-2,100) != -2){
        return -3;
    }if(utils::min(-10,-3) != -10){
        return -4;
    }if(utils::min(10,10) != 10){
        return -5;
    }
    return 0;
}
int max_check_utils(){ 
    if(utils::max(10,9) != 10){
        return -1;
    }if(utils::max(9,10) != 10){
        return -2;
    }if(utils::max(-2,100) != 100){
        return -3;
    }if(utils::max(-10,-3) != -3){
        return -4;
    }if(utils::max(10,10) != 10){
        return -5;
    }
    return 0;
}