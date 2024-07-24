#pragma once

#include <vector>
#include <unordered_map>
#include <iostream>

#include "ItemType.hpp"
#include "ObjectType.hpp"
#include "BuildRecipes.hpp"

class Inventory
{
    Inventory() = delete;

public:
    static void addItem(ItemType item, int amount);

    static void takeItem(ItemType item, int amount);

    static bool canBuildObject(ObjectType object);

    static std::unordered_map<ItemType, int> getTotalItemCount();

    inline static const std::vector<std::pair<ItemType, int>>& getData() {return inventoryData;}

private:
    static std::vector<std::pair<ItemType, int>> inventoryData;

};