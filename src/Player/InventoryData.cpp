#include "Player/InventoryData.hpp"

InventoryData::InventoryData(int size)
{
    inventoryData = std::vector<std::optional<ItemCount>>(size, std::nullopt);
}

void InventoryData::addItem(ItemType item, int amount)
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

void InventoryData::takeItem(ItemType item, int amount)
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
    }
}

void InventoryData::addItemAtIndex(int index, ItemType item, int amount)
{
    if (index >= inventoryData.size())
        return;
    
    std::optional<ItemCount>& itemSlot = inventoryData[index];

    // Attempt to add to stack
    if (itemSlot.has_value())
    {
        ItemCount& itemCount = itemSlot.value();

        // Item to add is same as item at index, so add
        if (item == itemCount.first)
        {
            itemCount.second = std::min(itemCount.second + amount, INVENTORY_STACK_SIZE);
        }

        return;
    }

    // Create new stack
    ItemCount itemCount;
    itemCount.first = item;
    itemCount.second = std::min(amount, static_cast<int>(INVENTORY_STACK_SIZE));

    itemSlot = itemCount;
}

void InventoryData::takeItemAtIndex(int index, int amount)
{
    if (index >= inventoryData.size())
        return;
    
    std::optional<ItemCount>& itemSlot = inventoryData[index];
    if (!itemSlot.has_value())
        return;
    
    ItemCount& itemCount = itemSlot.value();

    if (amount >= itemCount.second)
    {
        itemSlot = std::nullopt;
    }
    else
    {
        itemCount.second -= amount;
    }
}

std::unordered_map<ItemType, unsigned int> InventoryData::getTotalItemCount()
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

std::optional<ItemCount>& InventoryData::getItemSlotData(int index)
{
    assert(index < inventoryData.size());

    std::optional<ItemCount>& itemSlotData = inventoryData.at(index);

    return itemSlotData;
}