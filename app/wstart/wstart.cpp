#include <kern/file.h>
#include <kern/kernel_util.h>
#include <kern/process_buffer.h>
#include <kern/process_message.h>
#include <kern/syscall.h>
#include <utils/json_parser.h>

#include <stdio.h>
int main(int argc, char **argv)
{
    sys::file f = sys::file("initfs/startup.json");
    uint8_t *t = new uint8_t[f.get_file_length() + 2];
    f.read(t, f.get_file_length());
    t[f.get_file_length() + 1] = 0;

    utils::json_data js;
    js.from_data((char *)t);
    while (true)
    {
    }
    return 0;
}
