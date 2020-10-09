#include <time.h>




time_t time(time_t* second){
    if(second != nullptr){
        return 0;
    }else{
        return sys::get_time_total_sec();
    }
}
