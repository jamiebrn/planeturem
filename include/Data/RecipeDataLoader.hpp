#pragma once

#include <fstream>
#include <unordered_map>
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

    // static const std::vector<RecipeData>& getRecipeData();

    static const RecipeData& getRecipeData(uint64_t hash);

    static bool recipeHashExists(uint64_t hash);

    static const std::unordered_map<uint64_t, RecipeData>& getRecipeDataMap();

    static uint64_t getRecipeCount();

private:
    // static std::vector<RecipeData> loaded_recipeData;
    static std::unordered_map<uint64_t, RecipeData> loaded_recipeData;

};