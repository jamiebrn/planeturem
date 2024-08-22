#pragma once

#include <map>

#include "Data/ItemData.hpp"

struct RecipeData
{
    ItemType product;
    unsigned int productAmount = 1;

    std::map<ItemType, unsigned int> itemRequirements;
    
    std::string craftingStationRequired = "";
    int craftingStationLevelRequired;
};