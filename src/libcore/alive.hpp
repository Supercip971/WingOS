#pragma once

#include "libcore/fmt/log.hpp"
#include "libcore/str.hpp"
namespace core
{
class Alive
{

    core::Str _name;
    size_t _tick = 0;

public:
    Alive(core::Str name) : _name(name)
    {
    }

    void tick()
    {
        _tick++;
        if (_tick == 1000)
        {
            log::log$("Alive: {}", _name);
            _tick = 0;
        }
    }
};
} // namespace core