#pragma once

#include <map>
#include <optional>
#include <vector>

#include "Data/ItemData.hpp"

struct RecipeData
{
    ItemType product;
    unsigned int productAmount = 1;

    std::map<ItemType, unsigned int> itemRequirements;

    // Some recipes can only be seen if player has certain items,
    // rather than default behaviour of player seeing recipe if they have just one of the items required
    // Only requires one of key items to see recipe
    std::optional<std::vector<ItemType>> keyItems;
    
    std::string craftingStationRequired = "";
    int craftingStationLevelRequired;
};