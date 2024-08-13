#include "Data/BuildRecipeLoader.hpp"

std::map<std::string, std::map<unsigned int, BuildRecipe>> BuildRecipeLoader::loaded_buildRecipeData;

bool BuildRecipeLoader::loadData(std::string buildRecipeDataPath)
{
    std::ifstream file(buildRecipeDataPath);
    nlohmann::json data = nlohmann::json::parse(file);

    // Load data
    for (nlohmann::json::iterator iter = data.begin(); iter != data.end(); ++iter)
    {
        std::string recipeCategory = iter.key();

        for (nlohmann::json::iterator itemIter = iter.value().begin(); itemIter != iter.value().end(); ++itemIter)
        {
            BuildRecipe buildRecipeData;
            auto jsonBuildRecipeData = itemIter.value();

            for (nlohmann::json::iterator recipeIter = jsonBuildRecipeData.begin(); recipeIter != jsonBuildRecipeData.end(); ++recipeIter)
            {
                buildRecipeData.itemRequirements[std::stoi(recipeIter.key())] = static_cast<int>(recipeIter.value());
            }

            loaded_buildRecipeData[recipeCategory][std::stoi(itemIter.key())] = buildRecipeData;
        }
    }

    return true;
}

const BuildRecipe& BuildRecipeLoader::getBuildRecipe(unsigned int objectType)
{
    for (auto iter = loaded_buildRecipeData.begin(); iter != loaded_buildRecipeData.end(); iter++)
    {
        if (iter->second.count(objectType) <= 0)
            continue;
        
        return iter->second[objectType];
    }

    // Return first recipe from first category by default (failsafe)
    return getBuildRecipe(0, 0);
}

const BuildRecipe& BuildRecipeLoader::getBuildRecipe(int index, int categoryIndex)
{
    const std::string& categoryString = getCategoryString(categoryIndex);
    return loaded_buildRecipeData[categoryString][index];
}

unsigned int BuildRecipeLoader::getRecipeProductObject(int index, int categoryIndex)
{
    const std::string& categoryString = getCategoryString(categoryIndex);
    std::map<unsigned int, BuildRecipe>::const_iterator iter = BuildRecipeLoader::getBuildRecipeData().at(categoryString).begin();
    std::advance(iter, index);
    return iter->first;
}

const std::string& BuildRecipeLoader::getCategoryString(int categoryIndex)
{
    std::map<std::string, std::map<unsigned int, BuildRecipe>>::const_iterator iter = BuildRecipeLoader::getBuildRecipeData().begin();
    std::advance(iter, categoryIndex);
    return iter->first;
}