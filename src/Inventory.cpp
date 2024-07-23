#include "Inventory.hpp"

std::vector<std::pair<ItemType, int>> Inventory::inventoryData;

void Inventory::addItem(ItemType item, int amount)
{
    inventoryData.push_back({item, amount});
    std::cout << "added " << amount << " items to inventory" << std::endl;
}