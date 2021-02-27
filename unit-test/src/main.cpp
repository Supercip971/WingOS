
#include <utils/memory.h>
#include <plug/system_plug.h>
#include <stdio.h>
#include <string.h>
int main(int argc, char **argv){
    plug::init();
    plug::debug_out("hello  world", strlen("hello world"));
    return 0;
}