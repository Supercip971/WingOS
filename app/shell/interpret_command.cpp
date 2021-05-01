#include "interpret_command.h"
#include <kern/file.h>
#include <kern/kernel_util.h>
#include <kern/syscall.h>
#include <stdio.h>
#include <string.h>
#include <utils/sys/proc_info_flag.h>
#include <utils/wstring.h>
static constexpr size_t temp_buffer_size = 32;
size_t record_output()
{
    return 0;
}

void update_out_output(sys::file &outbuffer, char *temp_buffer)
{
    memset(temp_buffer, 0, temp_buffer_size);
    while (outbuffer.get_cursor_pos() < outbuffer.get_file_length() - 1)
    {
        size_t result = outbuffer.read((uint8_t *)temp_buffer, temp_buffer_size - 1);
        temp_buffer[result] = 0;
        printf(temp_buffer);
    }
}
void update_in_input(sys::file &inbuffer)
{
    while (sys::stdin.get_cursor_pos() < sys::stdin.get_file_length() - 1)
    {
        char temp = 0;
        sys::stdin.read((uint8_t *)&temp, 1);
        inbuffer.write((uint8_t *)&temp, 1);
    }
}
size_t start_programm(const char *path)
{

    sys::file inbuffer;
    sys::file outbuffer;
    auto pid = sys::start_programm(path);

    inbuffer = sys::get_process_stdf(3, pid);
    outbuffer = sys::get_process_stdf(1, pid);
    char *echo_buffer = new char[temp_buffer_size];
    while (true)
    {
        int status = 0;
        int ret = 0;
        sys::sys$get_process_info(pid, PROC_INFO_STATUS, &ret, &status);
        if (status == -2)
        {
            free(echo_buffer);
            return ret;
        }
        update_out_output(outbuffer, echo_buffer);
        update_in_input(inbuffer);
    }

    return 0;
}
int interpret_command(char *v)
{
    if (strcmp(v, "exit") == 0)
    {
        return INTERPRET_QUIT;
    }
    else
    {
        char *target_str = new char[2048];
        memset(target_str, 0, 2048);
        for (size_t i = 0; i < strlen(v); i++)
        {
            if (v[i] == ' ')
            {
                target_str[i] = 0;
                break;
            }
            target_str[i] = v[i];
        }
        printf("running: %s \n", target_str);
        sys::file target = sys::file(target_str);
        if (target.is_openned())
        {

            target.close();
            size_t res = start_programm(target_str);
            delete[] target_str;

            return res;
        }
        else
        {
        }
        delete[] target_str;
    }

    printf("error: command %s not available \n", v);
    return -1; // un handled command
}
