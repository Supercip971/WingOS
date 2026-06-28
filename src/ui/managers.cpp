#include "font-manager.hpp"
#include "image-manager.hpp"
fc::FontsRepo font_repo = {};

fc::FontsRepo &fc::FontsRepo::the()
{
    return font_repo;
}

fc::TextureRepo texture_repo = {};

fc::TextureRepo &fc::TextureRepo::the()
{
    return texture_repo;
}
