#pragma once

namespace hw
{

struct MouseEvent
{
    int absx, absy;
    int offx, offy, scroll;
    bool left, right, middle;
};
} // namespace hw
