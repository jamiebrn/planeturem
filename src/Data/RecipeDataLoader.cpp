#include "Data/RecipeDataLoader.hpp"

std::vector<RecipeData> RecipeDataLoader::loaded_recipeData;

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
                recipeData.itemRequirements[ItemDataLoader::getItemTypeFromName(itemIter.key())] = itemIter.value();
            }
        }

        if (jsonRecipeData.contains("crafting-station")) recipeData.craftingStationRequired = jsonRecipeData.at("crafting-station");
        if (jsonRecipeData.contains("crafting-station-level")) recipeData.craftingStationLevelRequired = jsonRecipeData.at("crafting-station-level");

        loaded_recipeData.push_back(recipeData);
    }

    return true;
}

const std::vector<RecipeData>& RecipeDataLoader::getRecipeData()
{
    return loaded_recipeData;
}