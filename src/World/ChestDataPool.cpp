#include "World/ChestDataPool.hpp"

ChestDataPool::ChestDataPool()
{
    // Reserve 0xFFFF - 1 slots for chest data
    // chestData = std::vector<std::optional<InventoryData>>;

    openDataSlots.clear();

    // topDataSlot = 0;
}

// ID 0xFFFF returned means chest was not initialised, as data is full
uint16_t ChestDataPool::createChest(const InventoryData& chestContents)
{
    // uint16_t chestID = topDataSlot;
    if (openDataSlots.size() > 0)
    {
        uint16_t chestID = openDataSlots.back();
        openDataSlots.pop_back();
        chestData[chestID] = chestContents;
    }
    else
    {
        if (chestData.size() < 0xFFFF)
        {
            chestData.push_back(chestContents);
            uint16_t chestID = chestData.size() - 1;
            return chestID;
        }
    }

    // if (chestID < chestData.size())
    // {
    //     // Initialise chest data
    //     chestData[chestID] = chestContents;
    // }

    return 0xFFFF;
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