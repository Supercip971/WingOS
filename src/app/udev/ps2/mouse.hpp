#pragma once
#include "controller.hpp"
#include "libcore/ds/vec.hpp"

namespace Ps2
{

typedef enum
{
    PS2_MOUSE_CMD_RESET = 0xff,
    PS2_MOUSE_CMD_RESEND = 0xfe,
    PS2_MOUSE_CMD_SET_DEFAULT = 0xf6,
    PS2_MOUSE_CMD_DISABLE_REPORT = 0xf5,
    PS2_MOUSE_CMD_ENABLE_REPORT = 0xf4,
    PS2_MOUSE_CMD_SET_RATE = 0xf3,
    PS2_MOUSE_CMD_DEV_ID = 0xf2,
    PS2_MOUSE_CMD_REMOTE_MODE = 0xf0,
    PS2_MOUSE_CMD_WRAP_MODE = 0xee,
    PS2_MOUSE_CMD_WRAP_RESET = 0xec,
    PS2_MOUSE_CMD_READ_DATA = 0xeb,
    PS2_MOUSE_CMD_SET_STREAM_MODE = 0xea,
    PS2_MOUSE_CMD_STATUS_REQUEST = 0xe9,
    PS2_MOUSE_CMD_SET_RESOLUTION = 0xe8,
    PS2_MOUSE_CMD_SET_SCALING = 0xe6,
} Ps2MouseCommands;

struct MouseEvent
{
    int offx, offy, scroll;
    bool left, right, middle;
};

class Mouse
{
    Controller &_controller;
    bool has_wheel;
    size_t cycle;
    core::Vec<MouseEvent> _events;
    uint8_t buf[4];

public:
    Mouse(Controller &controller) : _controller(controller)
    {
    }

    core::Result<MouseEvent> poll_event()
    {
        if (_events.len() == 0)
        {
            return "no event";
        }

        return _events.pop(0);
    }

    void send(uint8_t data)
    {
        _controller.port2_write(data);
        _controller.data_read();
    }

    void init()
    {
        _controller.port2_init();

        send(PS2_MOUSE_CMD_SET_DEFAULT);
        send(PS2_MOUSE_CMD_ENABLE_REPORT);

        send(0xF2); // enable scroll wheel
        _controller.data_read();

        send(PS2_MOUSE_CMD_SET_RATE);
        send(200);
        send(PS2_MOUSE_CMD_SET_RATE);
        send(100);
        send(PS2_MOUSE_CMD_SET_RATE);
        send(80);

        send(0xF2);

        uint8_t status = _controller.data_read();

        has_wheel = (status == 3);
        cycle = 0;
    }

    void handle_packet_group()
    {

        cycle = 0;

        MouseEvent ev;

        int flags = buf[0];
        ev.offx = (uint8_t)buf[1];
        ev.offy = (uint8_t)buf[2];

        if (ev.offx && (flags & (1 << 4)))
        {
            ev.offx -= 0x100;
        }

        if (ev.offy && (flags & (1 << 5)))
        {
            ev.offy -= 0x100;
        }

        if (has_wheel)
        {
            ev.scroll = (int8_t)buf[3];
        }
        else
        {
            ev.scroll = 0;
        }

        ev.left = flags & (1 << 0);
        ev.right = flags & (1 << 1);
        ev.middle = flags & (1 << 2);

        _events.push(ev);
    }


    bool handle_event()
    {
        bool acquired = false;
        uint8_t status = _controller.status();

        while (status & Controller::PS2_OUTPUT_STATUS_FULL && (status & 0x20))
        {

            uint8_t packet = _controller.data_read();
            handle_packet(packet);

            status = _controller.status();
            acquired = true;
        }

        return acquired;
    }

    void handle_packet(uint8_t packet)
    {
        switch (cycle)
        {
        case 0:
        {
            buf[0] = packet;
            if (packet & 8)
            {
                cycle++;
            }
            break;
        }
        case 1:
        {
            buf[1] = packet;
            cycle++;
            break;
        }
        case 2:
        {
            buf[2] = packet;
            if (has_wheel)
            {
                cycle++;
            }
            else
            {
                handle_packet_group();
            }
            break;
        }
        case 3:
        {
            buf[3] = packet;
            handle_packet_group();
            break;
        }
        default:
            cycle = 0;
            break;
        }
    }
};
} // namespace Ps2