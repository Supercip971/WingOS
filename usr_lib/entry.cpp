#include <klib/file.h>
extern "C" int main(int argc, char **argv);

extern "C" void __entry_point(int argc, char **argv)
{
    main(argc, argv);
}
