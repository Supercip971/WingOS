#include "shell.h"
#include <klib/kernel_util.h>
#include <klib/process_message.h>

#include <string.h>
namespace sys
{
    shell::shell()
    {
        shell_pid = start_programm("init_fs/shell.exe");
    }
    void shell::exit()
    {
        shell_protocol prot;
        prot.action = SHELL_EXIT;
        //process_message_pid((uint64_t)shell_pid, (uint64_t)&prot, sizeof(shell_protocol)).read();
    }
    void shell::send_key_tap(char key)
    {
        shell_protocol prot;
        prot.action = SHELL_SEND_KEY;
        prot.key.keycode = key;
        //process_message_pid((uint64_t)shell_pid, (uint64_t)&prot, sizeof(shell_protocol)).read();
    }
    void shell::send_command(const char *command)
    {
        shell_protocol prot;
        prot.action = SHELL_SEND_COMMAND;
        prot.command.command = new char[strlen(command) + 2];
        memcpy(prot.command.command, command, strlen(command) + 2);

        //process_message_pid((uint64_t)shell_pid, (uint64_t)&prot, sizeof(shell_protocol)).read();
        delete[] prot.command.command;
    }
} // namespace sys
