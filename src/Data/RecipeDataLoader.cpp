#include "Data/RecipeDataLoader.hpp"

std::map<ItemType, RecipeData> RecipeDataLoader::loaded_recipeData;

bool RecipeDataLoader::loadData(std::string recipeDataPath)
{
    std::ifstream file(recipeDataPath);
    nlohmann::json data = nlohmann::json::parse(file);

    // Load data
    // for (nlohmann::json::iterator iter = data.begin(); iter != data.end(); ++iter)
    // {
    //     loaded_furnaceRecipeData[std::stoi(iter.key())] = iter.value();
    // }

    return true;
}