

namespace wgfx
{
struct GRect
{
    long x;
    long y;
    long width;
    long height;

    constexpr void endx(long ex)
    {
        width = ex - x;
    }
    constexpr void endy(long ey)
    {
        height = ey - y;
    }
    constexpr long endx() const
    {
        return x + width;
    }
    constexpr long endy() const
    {
        return y + height;
    }

    constexpr GRect(long _x, long _y, long _width, long _height) : x(_x), y(_y), width(_width), height(_height) {}

    constexpr GRect() {};
    static constexpr GRect from_start_end(long sx, long sy, long ex, long ey)
    {
        return GRect(sx, sy, ex - sx, ey - sy);
    }
};
} // namespace wgfx
