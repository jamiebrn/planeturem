#include "Inventory.hpp"

std::vector<std::pair<ItemType, int>> Inventory::inventoryData;

void Inventory::addItem(ItemType item, int amount)
{
    std::cout << "added " << amount << " items to inventory" << std::endl;

    // Check if item already exists
    for (auto& itemPair : inventoryData)
    {
        if (itemPair.first == item && itemPair.second < 20)
        {
            int amount_added = std::min(itemPair.second + amount, 20) - itemPair.second;

            itemPair.second += amount_added;

            if (itemPair.second == 20 && amount - amount_added > 0)
                inventoryData.push_back({item, amount - amount_added});
            
            return;
        }
    }

    // Doesn't exist so add as new item
    inventoryData.push_back({item, amount});
}

bool Inventory::canBuildObject(ObjectType object)
{
    auto& recipeItemsRequired = BuildRecipes.at(object);

    std::unordered_map<ItemType, int> inventoryItemCount = getTotalItemCount();

    for (auto& recipeItemPair : recipeItemsRequired)
    {
        // If item not in inventory, return false (cannot build object)
        if (inventoryItemCount.count(recipeItemPair.first) <= 0)
            return false;
        
        // If item is in inventory, but not enough, return false (cannot build object)
        if (inventoryItemCount[recipeItemPair.first] < recipeItemPair.second)
            return false;
    }

    // Can build object as has passed tests
    return true;
}

std::unordered_map<ItemType, int> Inventory::getTotalItemCount()
{
    std::unordered_map<ItemType, int> itemCount;

    for (auto& itemPair : inventoryData)
    {
        // If item already in map, add to it
        if (itemCount.count(itemPair.first))
        {
            itemCount[itemPair.first] += itemPair.second;
            continue;
        }

        itemCount[itemPair.first] = itemPair.second;
    }

    return itemCount;
}