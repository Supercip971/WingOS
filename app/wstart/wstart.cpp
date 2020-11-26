#include <klib/file.h>
#include <klib/kernel_file_system.h>
#include <klib/kernel_util.h>
#include <klib/process_buffer.h>
#include <klib/process_message.h>
#include <klib/syscall.h>
#include <stdio.h>
int main()
{
    sys::file f = sys::file("init_fs/startup.json");
    uint8_t *t = new uint8_t[f.get_file_length() + 2];
    f.read(t, f.get_file_length());
    t[f.get_file_length() + 1] = 0;
    printf("startup.json = %s", t);
    while (true)
    {
    }
    return 0;
}
