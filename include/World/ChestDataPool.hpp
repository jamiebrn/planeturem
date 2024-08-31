#pragma once

#include <memory>
#include <array>
#include <vector>
#include <optional>

// #include "Player/Inventory.hpp

// typedef std::vector<std::optional<ItemCount>> ChestData;

// #include "World/ChestData.hpp"
#include "Player/InventoryData.hpp"

class ChestDataPool
{
public:
    ChestDataPool();

    // ID 0xFFFF returned means chest was not initialised, as data is full
    uint16_t createChest(int capacity);

    void destroyChest(uint16_t id);

    InventoryData& getChestData(uint16_t id);

    InventoryData* getChestDataPtr(uint16_t id);

private:
    // 0xFFFF reserved for uninitialised chest / null
    std::unique_ptr<std::array<std::optional<InventoryData>, 0xFFFF - 1>> chestData;

    std::vector<uint16_t> openDataSlots;

    uint16_t topDataSlot;
};