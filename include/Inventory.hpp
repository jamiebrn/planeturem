#pragma once

#include <vector>
#include <iostream>

#include "ItemType.hpp"

class Inventory
{
    Inventory() = delete;

public:
    static void addItem(ItemType item, int amount);

    inline static const std::vector<std::pair<ItemType, int>>& getData() {return inventoryData;}

private:
    static std::vector<std::pair<ItemType, int>> inventoryData;

};