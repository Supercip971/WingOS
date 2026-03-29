#pragma once


namespace wgfx
{
    class Vec2
    {
        public:
        float x;
        float y;

        constexpr Vec2(float x, float y) : x(x), y(y) {}

        constexpr Vec2() : x(0), y(0) {}



        constexpr Vec2 operator+(const Vec2& other) const {
            return Vec2(x + other.x, y + other.y);
        }
        constexpr Vec2 operator-(const Vec2& other) const {
            return Vec2(x - other.x, y - other.y);
        }
        constexpr Vec2 operator*(float scalar) const {
            return Vec2(x * scalar, y * scalar);
        }
        constexpr Vec2 operator/(float scalar) const {
            return Vec2(x / scalar, y / scalar);
        }

        constexpr bool operator==(const Vec2& other) const {
            return x == other.x && y == other.y;
        }
        constexpr bool operator!=(const Vec2& other) const {
            return x != other.x || y != other.y;
        }

        constexpr Vec2& operator+=(const Vec2& other) {
            x += other.x;
            y += other.y;
            return *this;
        }
        constexpr Vec2& operator-=(const Vec2& other) {
            x -= other.x;
            y -= other.y;
            return *this;
        }
        constexpr Vec2& operator*=(float scalar) {
            x *= scalar;
            y *= scalar;
            return *this;
        }
        constexpr Vec2& operator/=(float scalar) {
            x /= scalar;
            y /= scalar;
            return *this;
        }



    };
}
