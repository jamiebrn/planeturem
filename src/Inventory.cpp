#include "Inventory.hpp"

std::vector<std::pair<ItemType, int>> Inventory::inventoryData;

void Inventory::addItem(ItemType item, int amount)
{
    std::cout << "added " << amount << " items to inventory" << std::endl;

    // Check if item already exists
    for (auto& itemPair : inventoryData)
    {
        if (itemPair.first == item && itemPair.second < 10)
        {
            int amount_added = std::min(itemPair.second + amount, 10) - itemPair.second;

            itemPair.second += amount_added;

            if (itemPair.second == 10 && amount - amount_added > 0)
                inventoryData.push_back({item, amount - amount_added});
            
            return;
        }
    }

    // Doesn't exist so add as new item
    inventoryData.push_back({item, amount});
}