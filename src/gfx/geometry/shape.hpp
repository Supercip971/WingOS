#pragma once


#include "gfx/canvas/cmd.hpp"
#include "gfx/geometry/vec2.hpp"
#include "libcore/ds/vec.hpp"

namespace wgfx
{

enum class PathAction
{
    GMOVE,
    GPOINT,

    GCURVE,
    GCUBIC_CURVE
};

struct Curve
{
    Vec2 pos;
    Vec2 control;
};

struct CubicCurve
{
    Vec2 pos;
    Vec2 control1;
    Vec2 control2;
};

struct StrokePoint
{

    PathAction action;

    Vec2 pos;

    constexpr StrokePoint(PathAction action, const Vec2 &pos) : action(action), pos(pos) {}
    constexpr StrokePoint(const Curve &curve) : action(PathAction::GCURVE), pos(curve.pos), curve(curve) {}
    constexpr StrokePoint(const CubicCurve &cubic_curve) : action(PathAction::GCUBIC_CURVE), pos(cubic_curve.pos), cubic_curve(cubic_curve) {}

    constexpr static StrokePoint moved(const Vec2 &pos) { return StrokePoint(PathAction::GMOVE, pos); }
    constexpr static StrokePoint point(const Vec2 &pos) { return StrokePoint(PathAction::GPOINT, pos); }
    constexpr static StrokePoint curved(const Vec2 &pos, const Vec2 &control) { return StrokePoint(Curve{pos, control}); }
    constexpr static StrokePoint cubic_curved(const Vec2 &pos, const Vec2 &control1, const Vec2 &control2) { return StrokePoint(CubicCurve{pos, control1, control2}); }

    union
    {
        Curve curve;
        CubicCurve cubic_curve;
    };
};

class Contour
{
    GRect _bound = {};
    Vec2 off_pos = {};
    bool has_point = false;

    public:
    core::Vec<StrokePoint> strokes = {};

    void update_bound(Vec2 off)
    {
        if(!has_point)
        {
            _bound.start = off;
            _bound.end = off;
            has_point = true;
        }
        else
        {
            _bound = _bound.resized_to_contain(off);
        }
    }

    Contour() = default;

    GRect bound() const { return _bound; }

    Contour& stroke_move(const Vec2& offset)
    {
        strokes.push(StrokePoint::moved(offset));
        return *this;
    }

    Contour& stroke_point(const Vec2& offset)
    {
        update_bound(offset);
        strokes.push(StrokePoint::point(offset));
        return *this;
    }

    Contour& stroke_curve(const Vec2& offset, const Vec2& control)
    {
        update_bound(offset);
        strokes.push(StrokePoint::curved(offset, control));

        return *this;
    }

    Contour& stroke_cubic_curve(const Vec2& offset, const Vec2& control1, const Vec2& control2)
    {
        update_bound(offset);
        strokes.push(StrokePoint::cubic_curved(offset, control1, control2));

        return *this;
    }
};

} // namespace wgfx
