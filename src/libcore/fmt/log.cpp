

#include <libcore/io/writer.hpp>
#include <libcore/io/void.hpp>


#include <libcore/str.hpp>
namespace log 
{

    core::VoidRW default_target;
    core::Writer* target = &default_target; 
    void provide_log_target(core::Writer* writer)
    {
        target = writer;
    }

    core::Writer* log_target()
    {
        return target;
    }


}