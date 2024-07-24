#include "Player/Inventory.hpp"

std::vector<std::pair<ItemType, int>> Inventory::inventoryData;

void Inventory::addItem(ItemType item, int amount)
{
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

void Inventory::takeItem(ItemType item, int amount)
{
    int amount_left = amount;

    // Go backwards over inventory and subtract from item stacks
    for (int i = inventoryData.size() - 1; i >= 0; i--)
    {
        auto& itemPair = inventoryData[i];

        if (itemPair.first == item)
        {
            int amount_taken = std::min(itemPair.second, amount_left);

            // Must be taken from multiple stacks
            if (amount_taken < amount_left)
            {
                inventoryData.erase(inventoryData.begin() + i);
                amount_left -= amount_taken;
                continue;
            }

            // Stack has enough to take from
            itemPair.second -= amount_left;

            // Delete stack if now empty
            if (itemPair.second <= 0)
            {
                inventoryData.erase(inventoryData.begin() + i);
                amount_left -= amount_taken;
                continue;
            }
        }
    }
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