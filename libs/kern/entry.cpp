#include <kern/file.h>
extern "C" int main(int argc, char **argv);

extern "C" void __entry_point(int argc, char **argv)
{
    sys::stdin = sys::file("/dev/stdin");
    sys::stdout = sys::file("/dev/stdout");
    sys::stderr = sys::file("/dev/stderr");
    main(argc, argv);
}
