#pragma once

#include <map>

struct BuildRecipe
{
    std::map<unsigned int, unsigned int> itemRequirements;
};