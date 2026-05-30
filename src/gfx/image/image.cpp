#include "image.hpp"

#ifndef STBI_ASSERT
#    define STBI_ASSERT(x) ((void)(x))
#endif

#include <external/stb/stb_image.hpp>

#include "gfx/color.hpp"
namespace wgfx
{

template <>
core::Result<Image<Rgba01>> Image<Rgba01>::load_from_file(core::Str filename)
{
    int width, height, nrChannels;
    float*data = stbi_loadf(filename.data(), &width, &height, &nrChannels, 4);
    if (!data)
    {
        return "unable to load image";
    }

    Image<Rgba01> image = {};
    image._width = width;
    image._height = height;
    image._data = reinterpret_cast<Rgba01*>(data);
    return image;
}


} // namespace wgfx
