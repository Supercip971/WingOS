#include <gui/graphic_system.h>
#include <gui/window.h>
#include <kern/kernel_util.h>
#include <kern/mem_util.h>
#include <kern/process_message.h>
#include <kern/syscall.h>
//#include <feather_language_lib/feather.h>
#include <ctypes.h>
#include <gui/raw_graphic.h>
#include <gui/widget/button_widget.h>
#include <gui/widget/rectangle_widget.h>
#include <gui/widget/terminal_widget.h>
#include <kern/file.h>
#include <kern/process_buffer.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
size_t stdin_length;
char *temp_buffer;
size_t buffer_length;
char *next_command()
{
    int buffer_idx = 0;
    memset(temp_buffer, 0, buffer_length);
    while (true)
    {
        if (stdin_length < sys::stdin.get_file_length())
        {
            char target = 0;
            sys::stdin.read((uint8_t *)&target, 1);
            if (target == '\n')
            {
                temp_buffer[buffer_idx] = 0;
                stdin_length++;
                return temp_buffer;
            }
            else
            {

                temp_buffer[buffer_idx] = target;
                buffer_idx++;
            }
            stdin_length++;
        }
    }
}
int main(int argc, char **argv)
{
    stdin_length = sys::stdin.get_file_length();
    sys::stdin.seek(stdin_length);
    printf("wingos shell \n");
    printf("hello world ! \n");
    printf(">");
    int i = 0;
    temp_buffer = (char *)malloc(2048);
    buffer_length = 2048;
    while (true)
    {
        next_command();
        printf("%s", temp_buffer);
        printf("\n runned command %s", temp_buffer);
        printf("\n>");
    }
    free(temp_buffer);
    return 0;
}
