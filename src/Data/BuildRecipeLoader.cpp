#include "Data/BuildRecipeLoader.hpp"

std::map<unsigned int, BuildRecipe> BuildRecipeLoader::loaded_buildRecipeData;

bool BuildRecipeLoader::loadData(std::string buildRecipeDataPath)
{
    std::ifstream file(buildRecipeDataPath);
    nlohmann::json data = nlohmann::json::parse(file);

    // Load data
    for (nlohmann::json::iterator iter = data.begin(); iter != data.end(); ++iter)
    {
        BuildRecipe buildRecipeData;
        auto jsonBuildRecipeData = iter.value();

        for (nlohmann::json::iterator recipeIter = jsonBuildRecipeData.begin(); recipeIter != jsonBuildRecipeData.end(); ++recipeIter)
        {
            buildRecipeData.itemRequirements[std::stoi(recipeIter.key())] = static_cast<int>(recipeIter.value());
        }

        loaded_buildRecipeData[std::stoi(iter.key())] = buildRecipeData;
    }

    return true;
}

const BuildRecipe& BuildRecipeLoader::getBuildRecipe(int index)
{    
    return loaded_buildRecipeData[index];
}