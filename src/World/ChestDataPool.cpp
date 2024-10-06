#include "World/ChestDataPool.hpp"

ChestDataPool::ChestDataPool()
{
    // Reserve 0xFFFF - 1 slots for chest data
    chestData = std::vector<std::optional<InventoryData>>(0xFFFF - 1);

    openDataSlots.clear();

    topDataSlot = 0;
}

// ID 0xFFFF returned means chest was not initialised, as data is full
uint16_t ChestDataPool::createChest(InventoryData chestContents)
{
    uint16_t chestID = topDataSlot;
    if (openDataSlots.size() > 0)
    {
        chestID = openDataSlots.back();
        openDataSlots.pop_back();
    }
    else
    {
        if (topDataSlot < chestData.size())
        {
            topDataSlot++;
        }
    }

    if (chestID < chestData.size())
    {
        // Initialise chest data
        chestData[chestID] = chestContents;
    }

    return chestID;
}

uint16_t ChestDataPool::createChest(int capacity)
{
    return createChest(InventoryData(capacity));
}

void ChestDataPool::destroyChest(uint16_t id)
{
    if (id >= chestData.size())
    {
        return;
    }

    chestData[id] = std::nullopt;
    openDataSlots.push_back(id);
}

InventoryData& ChestDataPool::getChestData(uint16_t id)
{
    assert(id < chestData.size());

    return chestData.at(id).value();
}

InventoryData* ChestDataPool::getChestDataPtr(uint16_t id)
{
    if (id >= chestData.size())
    {
        return nullptr;
    }

    return &(chestData.at(id).value());
}