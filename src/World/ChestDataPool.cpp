#include "World/ChestDataPool.hpp"

ChestDataPool::ChestDataPool()
{
    chestData = std::make_unique<std::array<ChestData, 0xFFFF - 1>>();

    openDataSlots.clear();

    topDataSlot = 0;
}

// ID 0xFFFF returned means chest was not initialised, as data is full
uint16_t ChestDataPool::createChest(int capacity)
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
        chestData->at(chestID) = ChestData(capacity, std::nullopt);
    }

    return chestID;
}

void ChestDataPool::destroyChest(uint16_t id)
{
    if (id == 0xFFFF)
    {
        return;
    }

    chestData->at(id).clear();
    openDataSlots.push_back(id);
}

ChestData& ChestDataPool::getChestData(uint16_t id)
{
    assert(id != 0xFFFF);

    return chestData->at(id);
}

ChestData* ChestDataPool::getChestDataPtr(uint16_t id)
{
    if (id == 0xFFFF)
    {
        return nullptr;
    }

    return &(chestData->at(id));
}