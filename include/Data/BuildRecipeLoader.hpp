#pragma once

#include <SFML/Graphics.hpp>
#include <fstream>
#include <map>
#include <string>

#include "Core/json.hpp"
#include "Data/BuildRecipe.hpp"

class BuildRecipeLoader
{
    BuildRecipeLoader() = delete;

public:
    static bool loadData(std::string buildRecipeDataPath);

    static const BuildRecipe& getBuildRecipe(unsigned int objectType);

    static const BuildRecipe& getBuildRecipe(int index, int categoryIndex);

    static unsigned int getRecipeProductObject(int index, int categoryIndex);

    static const std::string& getCategoryString(int categoryIndex);

    inline static const std::map<unsigned int, BuildRecipe>& getBuildRecipeDataCategory(const std::string& category) {return loaded_buildRecipeData.at(category);}

    inline static const std::map<std::string, std::map<unsigned int, BuildRecipe>>& getBuildRecipeData() {return loaded_buildRecipeData;}

private:
    // String - category
    // Unsigned int - object ID of recipe product
    // Build recipe - items required and amounts
    static std::map<std::string, std::map<unsigned int, BuildRecipe>> loaded_buildRecipeData;

};