#pragma one

#include <SFML/Graphics.hpp>
#include <fstream>
#include <unordered_map>
#include <string>

#include "Core/json.hpp"

#include "Data/RecipeData.hpp"
#include "Data/ItemData.hpp"

class RecipeDataLoader
{
    RecipeDataLoader() = delete;

public:
    static bool loadData(std::string recipeDataPath);

private:
    static std::map<ItemType, RecipeData> loaded_recipeData;

};