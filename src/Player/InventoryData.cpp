#include "Player/InventoryData.hpp"

InventoryData::InventoryData(int size)
{
    inventoryData = std::vector<std::optional<ItemCount>>(size, std::nullopt);
}

int InventoryData::addItem(ItemType item, int amount)
{
    int amountToAdd = amount;

    // Attempt to add items to existing stacks
    for (std::optional<ItemCount>& itemSlot : inventoryData)
    {
        if (!itemSlot.has_value())
            continue;
        
        ItemCount& itemCount = itemSlot.value();

        const ItemData& itemData = ItemDataLoader::getItemData(itemCount.first);

        if (itemCount.first != item)
            continue;

        if (itemCount.second >= itemData.maxStackSize)
            continue;

        int amountAddedToStack = std::min(itemCount.second + amountToAdd, itemData.maxStackSize) - itemCount.second;

        amountToAdd -= amountAddedToStack;

        itemCount.second += amountAddedToStack;

        if (amountToAdd <= 0)
            return amount;
    }

    // Attempt to put remaining items in empty slot
    for (std::optional<ItemCount>& itemSlot : inventoryData)
    {
        if (itemSlot.has_value())
            continue;
        
        const ItemData& itemData = ItemDataLoader::getItemData(item);
        
        int amountPutInSlot = std::min(amountToAdd, static_cast<int>(itemData.maxStackSize));

        amountToAdd -= amountPutInSlot;

        itemSlot = ItemCount(item, amountPutInSlot);

        if (amountToAdd <= 0)
            return amount;
    }

    // Return total items added
    return amount - amountToAdd;

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

        const ItemData& itemData = ItemDataLoader::getItemData(itemCount.first);

        // Item to add is same as item at index, so add
        if (item == itemCount.first)
        {
            itemCount.second = std::min(itemCount.second + amount, itemData.maxStackSize);
        }

        return;
    }

    const ItemData& itemData = ItemDataLoader::getItemData(item);

    // Create new stack
    ItemCount itemCount;
    itemCount.first = item;
    itemCount.second = std::min(amount, static_cast<int>(itemData.maxStackSize));

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

std::unordered_map<ItemType, unsigned int> InventoryData::getTotalItemCount() const
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

bool InventoryData::isEmpty() const
{
    // Check all slots
    for (auto& itemSlot : inventoryData)
    {
        if (itemSlot.has_value())
        {
            return false;
        }
    }

    // Default case
    return true;
}

int InventoryData::getProjectileCountForWeapon(ToolType weapon) const
{
    const ToolData& toolData = ToolDataLoader::getToolData(weapon);

    if (toolData.toolBehaviourType != ToolBehaviourType::BowWeapon)
    {
        return 0;
    }

    int count = 0;

    // Iterate over inventory and count valid projectiles
    for (auto& itemSlot : inventoryData)
    {
        if (!itemSlot.has_value())
        {
            continue;
        }

        // Check projectile types for tool against this item
        for (ProjectileType projectileType : toolData.projectileShootTypes)
        {
            if (projectileType == ItemDataLoader::getItemData(itemSlot->first).projectileType)
            {
                count += itemSlot->second;
                break;
            }
        }
    }

    return count;
}