#include "Player/Inventory.hpp"

std::array<std::optional<ItemCount>, MAX_INVENTORY_SIZE> Inventory::inventoryData;

void Inventory::addItem(ItemType item, int amount)
{
    int amountToAdd = amount;

    // Attempt to add items to existing stacks
    for (std::optional<ItemCount>& itemSlot : inventoryData)
    {
        if (!itemSlot.has_value())
            continue;
        
        ItemCount& itemCount = itemSlot.value();

        if (itemCount.first != item)
            continue;

        if (itemCount.second >= INVENTORY_STACK_SIZE)
            continue;

        int amountAddedToStack = std::min(itemCount.second + amountToAdd, INVENTORY_STACK_SIZE) - itemCount.second;

        amountToAdd -= amountAddedToStack;

        itemCount.second += amountAddedToStack;

        if (amountToAdd <= 0)
            return;
    }

    // Attempt to put remaining items in empty slot
    for (std::optional<ItemCount>& itemSlot : inventoryData)
    {
        if (itemSlot.has_value())
            continue;
        
        int amountPutInSlot = std::min(amountToAdd, static_cast<int>(INVENTORY_STACK_SIZE));

        amountToAdd -= amountPutInSlot;

        itemSlot = ItemCount(item, amountPutInSlot);

        if (amountToAdd <= 0)
            return;
    }

    // Doesn't exist so add as new item
    // inventoryData.push_back({item, amount});
}

void Inventory::takeItem(ItemType item, int amount)
{
    int amountToTake = amount;

    // Go backwards over inventory and subtract from item stacks
    for (int i = inventoryData.size() - 1; i >= 0; i--)
    {
        std::optional<ItemCount>& itemSlot = inventoryData[i];

        if (!itemSlot.has_value())
            continue;
        
        ItemCount& itemCount = itemSlot.value();

        if (itemCount.first != item)
            continue;

        int amountTaken = std::min(static_cast<int>(itemCount.second), amountToTake);

        amountToTake -= amountTaken;

        if (itemCount.second <= amountTaken)
        {
            itemSlot = std::nullopt;
        }
        else
        {
            itemCount.second -= amountTaken;
        }

        if (amountToTake <= 0)
            return;

        // Must be taken from multiple stacks
        // if (amount_taken < amount_left)
        // {
        //     inventoryData.erase(inventoryData.begin() + i);
        //     amount_left -= amount_taken;
        //     continue;
        // }

        // // Stack has enough to take from
        // itemPair.second -= amount_left;

        // // Delete stack if now empty
        // if (itemPair.second <= 0)
        // {
        //     inventoryData.erase(inventoryData.begin() + i);
        //     amount_left -= amount_taken;
        // }

        // // Removing items completed
        // return;
    }
}

bool Inventory::canBuildObject(ObjectType object)
{
    const BuildRecipe& buildRecipe = BuildRecipeLoader::getBuildRecipe(object);

    std::unordered_map<ItemType, unsigned int> inventoryItemCount = getTotalItemCount();

    for (auto& recipeItemPair : buildRecipe.itemRequirements)
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

std::unordered_map<ItemType, unsigned int> Inventory::getTotalItemCount()
{
    std::unordered_map<ItemType, unsigned int> totalItemCount;

    for (const std::optional<ItemCount>& itemSlot : inventoryData)
    {
        if (!itemSlot.has_value())
            continue;
        
        const ItemCount& itemCount = itemSlot.value();

        // If item already in map, add to it
        if (totalItemCount.count(itemCount.first))
        {
            totalItemCount[itemCount.first] += itemCount.second;
            continue;
        }

        totalItemCount[itemCount.first] = itemCount.second;
    }

    return totalItemCount;
}