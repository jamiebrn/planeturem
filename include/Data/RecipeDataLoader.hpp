#pragma once

#include <SFML/Graphics.hpp>
#include <fstream>
#include <vector>
#include <string>
#include <iostream>

#include "Core/json.hpp"

#include "Data/RecipeData.hpp"
#include "Data/ItemData.hpp"
#include "Data/ItemDataLoader.hpp"

class RecipeDataLoader
{
    RecipeDataLoader() = delete;

public:
    static bool loadData(std::string recipeDataPath);

    static const std::vector<RecipeData>& getRecipeData();

private:
    static std::vector<RecipeData> loaded_recipeData;

};