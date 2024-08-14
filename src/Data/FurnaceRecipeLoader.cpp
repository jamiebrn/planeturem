#include "Data/FurnaceRecipeLoader.hpp"

std::unordered_map<unsigned int, unsigned int> FurnaceRecipeLoader::loaded_furnaceRecipeData;

bool FurnaceRecipeLoader::loadData(std::string objectDataPath)
{
    std::ifstream file(objectDataPath);
    nlohmann::json data = nlohmann::json::parse(file);

    // Load data
    for (nlohmann::json::iterator iter = data.begin(); iter != data.end(); ++iter)
    {
        loaded_furnaceRecipeData[std::stoi(iter.key())] = iter.value();
    }

    return true;
}