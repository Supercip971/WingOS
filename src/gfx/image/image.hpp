#pragma once
#include <stddef.h>
#include <stdint.h>

#include "gfx/color.hpp"

namespace wgfx
{

template <typename Format>
class Image
{
public:
    Image(const Image &other) = delete;
    Image &operator=(const Image &other) = delete;

    Format *_data;
    size_t _width;
    size_t _height;

    size_t width() const { return _width; }

    size_t height() const { return _height; }

    Format *data() const { return _data; }

    Image() : _data(nullptr), _width(0), _height(0) {}

    static Image own(size_t width, size_t height, Format *data)
    {
        Image result;
        result._width = width;
        result._height = height;
        result._data = data;
        return result;
    }

    Image(Image &&other) noexcept
    {
        _data = other._data;
        _width = other._width;
        _height = other._height;
        other._data = nullptr;
    }

    Image &operator=(Image &&other) noexcept
    {
        if (this != &other)
        {
            _data = other._data;
            _width = other._width;
            _height = other._height;
            other._data = nullptr;
        }
        return *this;
    }

    ~Image()
    {
        if (_data)
        {
            free(_data);
            _data = nullptr;
        }
    }

    template <typename OtherFormat = Format>
    Image<OtherFormat> copy();
    static core::Result<Image<Format>> load_from_file(core::Str filename);
};

using ImageRGBA8 = Image<Rgba8>;
using Image01 = Image<Rgba01>;

template <>
template <>
inline Image<Rgba8> Image<Rgba01>::copy<Rgba8>()
{
    ImageRGBA8 result;
    result._data = static_cast<Rgba8 *>(malloc(_width * _height * sizeof(Rgba8)));
    result._width = _width;
    result._height = _height;
    for (size_t i = 0; i < _width * _height; i++)
    {
        result._data[i] = Rgba8::from_01a(_data[i].r, _data[i].g, _data[i].b, _data[i].a);
    }
    return result;
}

template <>
template <>
inline Image<Rgba01> Image<Rgba01>::copy<Rgba01>()
{
    Image01 result;
    result._data = static_cast<Rgba01 *>(malloc(_width * _height * sizeof(Rgba01)));
    result._width = _width;
    result._height = _height;
    for (size_t i = 0; i < _width * _height; i++)
    {
        result._data[i] = _data[i];
    }
    return result;
}

} // namespace wgfx
