#include <kern/file.h>
#include <lcxxabi.h>
#include <stdio.h>
#include <stdlib.h>
extern "C" int main(int argc, char **argv);
#ifdef MODULE

#include <module/module_calls.h>
extern "C" void __entry_point(int argc, char **argv, char **env)
{

    call_init();
    int res = main(argc, argv);
    __cxa_finalize(nullptr);
}
#else

asm(
    ".global __entry_point \n"
    "__entry_point: \n"
    "   and rsp, -16 \n"
    "   sub rsp, 512 \n"
    "   call _entry_point \n"
    "   int 0x1 \n"
);
extern "C" void _entry_point(int argc, char **argv, char **env)
{
    asm volatile("and rsp, -16");
    sys::stdin = sys::file("/dev/stdin");
    sys::stdout = sys::file("/dev/stdout");
    sys::stderr = sys::file("/dev/stderr");

    stdin = (FILE *)malloc(sizeof(FILE));
    stdin->file_element = sys::stdin.get_fid();

    stdout = (FILE *)malloc(sizeof(FILE));
    stdout->file_element = sys::stdout.get_fid();

    stderr = (FILE *)malloc(sizeof(FILE));
    stderr->file_element = sys::stderr.get_fid();

    int res = main(argc, argv);

    __cxa_finalize(nullptr);
    free(stdin);
    free(stdout);
    free(stderr);
    exit(res);
}
#endif
