#pragma once

#include <vector>
#include <iostream>

enum ItemType
{
    Wood
};

class Inventory
{
    Inventory() = delete;

public:
    static void addItem(ItemType item, int amount);

private:
    static std::vector<std::pair<ItemType, int>> inventoryData;

};