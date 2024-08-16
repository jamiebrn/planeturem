#pragma once

#include <SFML/Graphics.hpp>
#include <fstream>
#include <map>
#include <string>

#include "Core/json.hpp"
#include "Data/BuildRecipe.hpp"

#include "Data/ObjectDataLoader.hpp"

class BuildRecipeLoader
{
    BuildRecipeLoader() = delete;

public:
    static bool loadData(std::string buildRecipeDataPath);

    static const BuildRecipe& getBuildRecipe(ObjectType objectType);

    // Iterative methods used to find build recipe data via an index, rather than object type
    // Used in build GUI to increment through recipes per category
    static const BuildRecipe& getBuildRecipe(int index, int categoryIndex);
    static ObjectType getRecipeProductObject(int index, int categoryIndex);
    static const std::string& getCategoryString(int categoryIndex);

    inline static const std::map<ObjectType, BuildRecipe>& getBuildRecipeDataCategory(const std::string& category) {return loaded_buildRecipeData.at(category);}

    inline static const std::map<std::string, std::map<ObjectType, BuildRecipe>>& getBuildRecipeData() {return loaded_buildRecipeData;}

private:
    // String - category
    // Unsigned int - object ID of recipe product
    // Build recipe - items required and amounts
    static std::map<std::string, std::map<ObjectType, BuildRecipe>> loaded_buildRecipeData;

};