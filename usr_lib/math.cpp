#include <math.h>
#include <stdint.h>
double pow(double x, double y){
    if(y == 0){
        return 1;
    }
    else if( y == 2){
        return x*x;
    }
    else if(y == 1){
        return x;
    }
    double res = 1;
    for(unsigned int count = 0; count < y; count++){
        res*= x;
    }
    return res;

}
float powf(float x, float y){
    if(y == 0){
        return 1;
    }
    else if( y == 2){
        return x*x;
    }
    else if(y == 1){
        return x;
    }
    float res = 1;
    for(unsigned int count = 0; count < y; count++){
        res*= x;
    }
    return res;
}
long double powl( long double x, long double y){
    if(y == 0){
        return 1;
    }
    else if( y == 2){
        return x*x;
    }
    else if(y == 1){
        return x;
    }
    long double res = 1;
    for(unsigned int count = 0; count < y; count++){
        res*= x;
    }
    return res;
}

int isinf(double x){
    uint64_t nbres = (uint64_t)x;
    return ((unsigned)(nbres >> 32) & 0x7fffffff) == 0x7ff00000 && ((unsigned) nbres == 0);
}
int isnan(double x){
    uint64_t nbres = (uint64_t)x;
    return ((unsigned)(nbres >> 32) & 0x7fffffff) + ((unsigned) nbres != 0) > 0x7ff00000;
}
