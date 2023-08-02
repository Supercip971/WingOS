#include <libcore/fmt/log.hpp>
int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    log::log$("Hello, World!");
    return 0;
}