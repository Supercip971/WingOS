#pragma once
#include "controller.hpp"
#include "libcore/ds/vec.hpp"
#include "stdint.h"
namespace Ps2
{

struct Ps2KeyboardEvent
{
    uint8_t key;
    bool down;
};
class Ps2Keyboard
{
    Controller &_controller;
    core::Vec<Ps2KeyboardEvent> _events;

    bool escape = false;

public:
    Ps2Keyboard(Controller &controller) : _controller(controller)
    {
    }

    void init()
    {
        _controller.port1_init();
    }

    core::Result<Ps2KeyboardEvent> poll_event()
    {
        if (_events.len() == 0)
        {
            return "no event";
        }

        return _events.pop(0);
    }

    bool packet_handle()
    {
        bool acquired = false;
        uint8_t status = _controller.status();

        while (status & Controller::PS2_OUTPUT_STATUS_FULL && !(status & 0x20))
        {

            uint8_t packet = _controller.data_read();

            if (escape)
            {
                escape = false;
                uint8_t key = (packet & 0x7f) + 0x80;
                Ps2KeyboardEvent ev = {
                    .key = key,
                    .down = !(packet & 0x80),
                };

                _events.push(ev);
            }
            else if (packet == 0xE0)
            {
                escape = true;
            }
            else
            {
                Ps2KeyboardEvent ev = {
                    .key = (uint8_t)(packet & 0x7f),
                    .down = !(packet & 0x80),
                };

                _events.push(ev);
            }

            status = _controller.status();
            acquired = true;
        }

        return acquired;
    };
};
} // namespace Ps2
