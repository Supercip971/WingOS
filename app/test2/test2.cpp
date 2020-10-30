#include <klib/syscall.h>
#include <klib/process_message.h>
#include <stdio.h>
#include <klib/kernel_file_system.h>
#include <klib/file.h>
int main(){

    printf("hallo world\n");

    sys::file f("init_fs/hello1.txt");
    printf("hallo world\n");
    char* data = new char[32];
    printf("start\n");

    data[32] = 0;
    f.seek(0);
    f.read((uint8_t*)data,30);
    printf("%s \n", data);
    printf("end\n");
    f.close();
    while (true) {
    }
    return 0;
}
