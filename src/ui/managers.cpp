#include "font-manager.hpp"
#include "image-manager.hpp"
static fc::FontsRepo font_repo = {};

fc::FontsRepo &fc::FontsRepo::the()
{
    return font_repo;
}

static fc::TextureRepo texture_repo = {};

fc::TextureRepo &fc::TextureRepo::the()
{
    return texture_repo;
}
