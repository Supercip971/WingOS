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

struct RawStroke
{
    StrokePoint a;
    StrokePoint b;
};

class Contour
{
    GRect _bound = {};
    Vec2 off_pos = {};
    bool has_point = false;
    bool _invalidated = true;
    StrokePoint last_stroke = StrokePoint::moved(Vec2(0, 0));

public:
    core::Vec<RawStroke> strokes = {};
    core::Vec<StrokePoint> commands = {};

    void update_bound(Vec2 off)
    {
        if (!has_point)
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

    void add_elt(StrokePoint point)
    {

        if (point.action == PathAction::GMOVE)
        {
            last_stroke = point;
            return;
        }

        update_bound(point.pos);

        strokes.add_sorted(
            [](RawStroke const &left, RawStroke const &right)
            { return core::min(left.a.pos.x, left.b.pos.x) - core::min(right.a.pos.x, right.b.pos.x); },
            RawStroke{last_stroke, point});
        last_stroke = point;
    }
    void compute_cache()
    {

        if (!_invalidated)
        {
            return;
        }

        _invalidated = false;

        strokes.clear();

        StrokePoint last = StrokePoint::moved(Vec2(0, 0));
        for (StrokePoint a : commands)
        {
            if (a.action == PathAction::GMOVE)
            {
                last = a;
                continue;
            }

            strokes.push(RawStroke{last, a});
            last = a;
        }

        strokes.quick_sort([](RawStroke const &left, RawStroke const &right)
                           { return left.a.pos.x - right.b.pos.x; }, 0, strokes.len());

        // now sorts
    }

    Contour() = default;

    GRect bound() const { return _bound; }

    Contour &stroke_move(const Vec2 &offset)
    {
        commands.push(StrokePoint::moved(offset));
        add_elt(commands.last());

        return *this;
    }

    Contour &stroke_point(const Vec2 &offset)
    {
        commands.push(StrokePoint::point(offset));

        add_elt(commands.last());

        return *this;
    }

    Contour &stroke_curve(const Vec2 &offset, const Vec2 &control)
    {

        commands.push(StrokePoint::curved(offset, control));

        add_elt(commands.last());

        return *this;
    }

    Contour &stroke_cubic_curve(const Vec2 &offset, const Vec2 &control1, const Vec2 &control2)
    {
        commands.push(StrokePoint::cubic_curved(offset, control1, control2));

        add_elt(commands.last());

        return *this;
    }
};

} // namespace wgfx
