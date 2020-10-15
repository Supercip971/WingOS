#include <interface.h>
#include <logging.h>
extern "C" void __cxa_pure_virtual()
{
    log("virtual", LOG_ERROR) << "error while trying to call a virtual function D:";
}
