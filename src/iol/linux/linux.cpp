

#include <libcore/fmt/log.hpp>
#include <stdio.h>
#include <stdlib.h>
class LinuxLogger : public core::Writer
{
public:
    virtual core::Result<void> write(const char *data, size_t size) override
    {

        size_t v = fwrite(data, 1, size, stdout);
        if (v != size)
        {
            return core::Result<void>("Failed to write to stdout");
        }
        return {};
    }
};

extern "C" int main(int argc, char **argv);
extern "C" int _linux_start(int argc, char **argv)
{
    printf("started\n");
    LinuxLogger logger;
    log::provide_log_target(&logger);

    int c = main(argc, argv);

    _exit(c);
}