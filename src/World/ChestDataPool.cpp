#include "World/ChestDataPool.hpp"

ChestDataPool::ChestDataPool()
{
    chestData = std::make_unique<std::array<std::optional<InventoryData>, 0xFFFF - 1>>();

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
        if (topDataSlot < 0xFFFF)
        {
            topDataSlot++;
        }
    }

    if (chestID < 0xFFFF)
    {
        // Initialise chest data
        chestData->at(chestID) = chestContents;
    }

    return chestID;
}

uint16_t ChestDataPool::createChest(int capacity)
{
    return createChest(InventoryData(capacity));
}

void ChestDataPool::destroyChest(uint16_t id)
{
    if (id == 0xFFFF)
    {
        return;
    }

    chestData->at(id) = std::nullopt;
    openDataSlots.push_back(id);
}

InventoryData& ChestDataPool::getChestData(uint16_t id)
{
    assert(id != 0xFFFF);

    return chestData->at(id).value();
}

InventoryData* ChestDataPool::getChestDataPtr(uint16_t id)
{
    if (id == 0xFFFF)
    {
        return nullptr;
    }

    return &(chestData->at(id).value());
}