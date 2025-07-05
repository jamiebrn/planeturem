#include "Data/RecipeDataLoader.hpp"
#include <extlib/hashpp.h>

std::unordered_map<uint64_t, RecipeData> RecipeDataLoader::loaded_recipeData;

std::string RecipeDataLoader::dataHash;

bool RecipeDataLoader::loadData(std::string recipeDataPath)
{
    std::ifstream file(recipeDataPath);
    nlohmann::ordered_json data = nlohmann::ordered_json::parse(file);

    // Load data
    for (nlohmann::ordered_json::iterator iter = data.begin(); iter != data.end(); ++iter)
    {
        RecipeData recipeData;
        auto jsonRecipeData = iter.value();

        recipeData.product = ItemDataLoader::getItemTypeFromName(jsonRecipeData.at("product"));
        if (jsonRecipeData.contains("product-amount")) recipeData.productAmount = jsonRecipeData.at("product-amount");

        if (jsonRecipeData.contains("item-requirements"))
        {
            auto itemRequirements = jsonRecipeData.at("item-requirements");
            for (auto itemIter = itemRequirements.begin(); itemIter != itemRequirements.end(); ++itemIter)
            {
                ItemType item = ItemDataLoader::getItemTypeFromName(itemIter.key());

                recipeData.itemRequirements[item] = itemIter.value();
                ItemDataLoader::setItemIsMaterial(item, true);
            }
        }

        if (jsonRecipeData.contains("key-items"))
        {
            auto keyItems = jsonRecipeData.at("key-items");

            recipeData.keyItems = std::vector<ItemType>();

            for (auto itemIter = keyItems.begin(); itemIter != keyItems.end(); ++itemIter)
            {
                ItemType item = ItemDataLoader::getItemTypeFromName(itemIter.value());
                recipeData.keyItems->push_back(item);
            }
        }

        if (jsonRecipeData.contains("crafting-station")) recipeData.craftingStationRequired = jsonRecipeData.at("crafting-station");
        if (jsonRecipeData.contains("crafting-station-level")) recipeData.craftingStationLevelRequired = jsonRecipeData.at("crafting-station-level");

        loaded_recipeData[recipeData.getHash()] = recipeData;
    }
    
    dataHash = hashpp::get::getFileHash(hashpp::ALGORITHMS::MD5, recipeDataPath);

    return true;
}

const RecipeData& RecipeDataLoader::getRecipeData(uint64_t hash)
{
    return loaded_recipeData.at(hash);
}

bool RecipeDataLoader::recipeHashExists(uint64_t hash)
{
    return loaded_recipeData.contains(hash);
}

const std::unordered_map<uint64_t, RecipeData>& RecipeDataLoader::getRecipeDataMap()
{
    return loaded_recipeData;
}

uint64_t RecipeDataLoader::getRecipeCount()
{
    return loaded_recipeData.size();
}