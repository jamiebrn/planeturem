#pragma once

#include <vector>
#include <unordered_map>

#include "Data/BuildRecipeLoader.hpp"

class Inventory
{
    Inventory() = delete;

public:
    static void addItem(unsigned int item, int amount);

    static void takeItem(unsigned int item, int amount);

    static bool canBuildObject(unsigned int object);

    static std::unordered_map<unsigned int, int> getTotalItemCount();

    inline static const std::vector<std::pair<unsigned int, int>>& getData() {return inventoryData;}

private:
    static std::vector<std::pair<unsigned int, int>> inventoryData;

};