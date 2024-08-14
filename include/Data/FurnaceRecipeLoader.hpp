#pragma one

#include <SFML/Graphics.hpp>
#include <fstream>
#include <unordered_map>
#include <string>

#include "Core/json.hpp"

class FurnaceRecipeLoader
{
    FurnaceRecipeLoader() = delete;

public:
    static bool loadData(std::string furnaceRecipeDataPath);

    static inline unsigned int getFurnaceProduct(unsigned int item) {return loaded_furnaceRecipeData.at(item);}

private:
    // Maps item put in furnace : item created from furnace
    static std::unordered_map<unsigned int, unsigned int> loaded_furnaceRecipeData;

};