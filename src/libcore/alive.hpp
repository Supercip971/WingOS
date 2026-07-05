#pragma once

#include "libcore/fmt/log.hpp"
#include "libcore/str.hpp"

namespace fc
{
class Alive
{

    fc::Str _name;
    size_t _tick = 0;

public:
    Alive(fc::Str name) : _name(name)
    {
    }

    void tick()
    {
        _tick++;
        if (_tick == 1000)
        {
            fmt::log$("Alive: {}", _name);
            _tick = 0;
        }
    }
};
} // namespace fc
