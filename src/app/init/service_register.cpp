#include "service_register.hpp"

#include "app/init/module_startup.hpp"

#include "iol/wingos/ipc.hpp"
#include "libcore/ds/vec.hpp"
#include "libcore/fmt/log.hpp"
#include "libcore/result.hpp"
#include "libcore/str.hpp"
#include "protocols/init/init.hpp"
#include "wingos-headers/ipc.h"

fc::Vec<Wingos::IpcConnection *> connections = {};

struct RegisteredService
{

    char name[80];
    uint64_t major;
    uint64_t minor;
    uint64_t endpoint;
};

fc::Vec<RegisteredService *> registered_services = {};

fc::Result<void> service_register(uint64_t endpoint, fc::Str const &name, uint64_t major, uint64_t minor)

{
    fmt::log$("registering service: {} ({}.{}) at {}", name, major, minor, endpoint);
    RegisteredService *service = new RegisteredService();
    size_t i;
    for (i = 0; i < 80 - 1 && i < name.len(); i++)
    {
        service->name[i] = name[i];
    }
    service->name[i] = 0;

    service->endpoint = endpoint;
    service->major = major;
    service->minor = minor;

    registered_services.push(service);

    fmt::log$("(server) registered service: {} ({}.{}) at {}", service->name, service->major, service->minor, service->endpoint);

    service_startup_callback(service->name);

    return {};
}

fc::Result<IpcServerHandle> service_get(fc::Str const &name, uint64_t major, uint64_t minor)
{
    for (size_t j = 0; j < registered_services.len(); j++)
    {
        auto &service = registered_services[j];
        bool name_match = true;

        name_match = (fc::Str(service->name) == name);
        if (name_match && service->major == major && service->minor >= minor)
        {
            return {service->endpoint};
        }
    }

    fmt::log$("service not found: {} ({}.{})", name, major, minor);

    return ("service not found");
}
