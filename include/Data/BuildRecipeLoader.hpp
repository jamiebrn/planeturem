#pragma once

#include <SFML/Graphics.hpp>
#include <fstream>
#include <map>

#include "Core/json.hpp"
#include "Data/BuildRecipe.hpp"

class BuildRecipeLoader
{
    BuildRecipeLoader() = delete;

public:
    static bool loadData(std::string buildRecipeDataPath);

    static const BuildRecipe& getBuildRecipe(int index);

    inline static const std::map<unsigned int, BuildRecipe>& getBuildRecipeData() {return loaded_buildRecipeData;}

private:
    static std::map<unsigned int, BuildRecipe> loaded_buildRecipeData;

};