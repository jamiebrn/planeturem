#pragma once

#include <memory>
#include <array>
#include <vector>
#include <optional>

#include <extlib/cereal/archives/binary.hpp>
#include <extlib/cereal/types/memory.hpp>
#include <extlib/cereal/types/array.hpp>
#include <extlib/cereal/types/vector.hpp>
#include <extlib/cereal/types/optional.hpp>

// #include "Player/Inventory.hpp

// typedef std::vector<std::optional<ItemCount>> ChestData;

// #include "World/ChestData.hpp"
#include "Player/InventoryData.hpp"

class ChestDataPool
{
public:
    ChestDataPool();
    ChestDataPool::ChestDataPool(const ChestDataPool& pool);
    ChestDataPool &ChestDataPool::operator=(const ChestDataPool& pool);

    // ID 0xFFFF returned means chest was not initialised, as data is full
    uint16_t createChest(InventoryData chestContents);
    uint16_t createChest(int capacity);

    void destroyChest(uint16_t id);

    InventoryData& getChestData(uint16_t id);

    InventoryData* getChestDataPtr(uint16_t id);

    template <class Archive>
    void serialize(Archive& ar)
    {
        ar(chestData, openDataSlots, topDataSlot);
    }

private:
    // 0xFFFF reserved for uninitialised chest / null
    std::unique_ptr<std::array<std::optional<InventoryData>, 0xFFFF - 1>> chestData;

    std::vector<uint16_t> openDataSlots;

    uint16_t topDataSlot;
};