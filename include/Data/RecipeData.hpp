#pragma once

#include <map>

#include "Data/ItemData.hpp"

struct RecipeData
{
    std::map<ItemType, unsigned int> itemRequirements;
    
    std::string craftingStationRequired;
    int craftingStationLevelRequired;
};