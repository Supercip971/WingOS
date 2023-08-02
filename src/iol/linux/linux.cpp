

#include <libcore/fmt/log.hpp>
#include <stdio.h>

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

    LinuxLogger logger;
    log::provide_log_target(&logger);

    return main(argc, argv);
}