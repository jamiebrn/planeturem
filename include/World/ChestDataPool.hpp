#pragma once

#include <vector>
#include <optional>

#include <extlib/cereal/archives/binary.hpp>
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

    // ID 0xFFFF returned means chest was not initialised, as data is full
    uint16_t createChest(InventoryData chestContents);
    uint16_t createChest(int capacity);

    void destroyChest(uint16_t id);

    InventoryData& getChestData(uint16_t id);

    InventoryData* getChestDataPtr(uint16_t id);

    template <class Archive>
    void serialize(Archive& ar)
    {
        ar(chestData, openDataSlots);
    }

private:
    // 0xFFFF reserved for uninitialised chest / null
    std::vector<std::optional<InventoryData>> chestData;

    std::vector<uint16_t> openDataSlots;

    uint16_t topDataSlot;
};