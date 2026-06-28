#pragma once

#include "gfx/color.hpp"
#include "gfx/geometry/rect.hpp"
#include "gfx/geometry/vec2.hpp"
#include "gfx/text/font.hpp"
#include "libcore/ds/vec.hpp"
#include "libcore/shared.hpp"
#include "widget.hpp"

namespace fc
{

template <typename State>
class Statefull : public Widget, public State
{

public:
    bool transferTo(Widget &other) override
    {
        static_cast<Statefull<State> &>(other).State::operator=(static_cast<const State &>(*this));
        other.childs = core::move(childs);
        return true;
    }

    virtual ~Statefull() override = default;
};
} // namespace fc
