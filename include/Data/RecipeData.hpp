#pragma once

#include <map>
#include <optional>
#include <vector>
#include <string>

#include "Data/ItemData.hpp"
#include "Data/ItemDataLoader.hpp"

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

    inline uint64_t getHash() const
    {
        std::string toHash = std::to_string(productAmount) + "x" + ItemDataLoader::getItemData(product).name + "=";
        for (auto iter = itemRequirements.begin(); iter != itemRequirements.end();)
        {
            toHash += std::to_string(iter->second) + "x" + ItemDataLoader::getItemData(iter->first).name;

            iter++;
            if (iter != itemRequirements.end())
            {
                toHash += "+";
            }
        }
        return std::hash<std::string>{}(toHash);
    }
};