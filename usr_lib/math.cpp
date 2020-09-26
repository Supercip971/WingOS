#include <math.h>
#ifdef SSE
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
#endif
