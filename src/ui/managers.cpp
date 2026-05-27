#include "font-manager.hpp"

fc::FontsRepo font_repo = {};

fc::FontsRepo &fc::FontsRepo::the()
{
    return font_repo;
}
