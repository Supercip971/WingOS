

#include <libcore/fmt/log.hpp>
#include <stdio.h>
#include <stdlib.h>

#include "libcore/io/writer.hpp"
#include "libcore/result.hpp"

class LinuxLogger : public fc::Writer
{
public:
    virtual fc::Result<void> write(const char *data, size_t size) override
    {

        size_t v = fwrite(data, 1, size, stdout);
        if (v != size)
        {
            return fc::Result<void>("Failed to write to stdout");
        }
        return {};
    }
};

extern "C" int main(int argc, char **argv);

extern "C" int _linux_start(int argc, char **argv)
{

    printf("started\n");
    LinuxLogger logger;
    fmt::provide_log_target(&logger);

    int c = main(argc, argv);

    exit(c);
    // return c;
}
