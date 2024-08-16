#pragma once

#include <map>

#include "Data/ItemDataLoader.hpp"

struct BuildRecipe
{
    // Stores how many of each type of item required for recipe
    std::map<ItemType, unsigned int> itemRequirements;
};