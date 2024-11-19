#pragma once

#include <vector>
#include <optional>
#include <unordered_map>

#include <extlib/cereal/archives/binary.hpp>
#include <extlib/cereal/types/vector.hpp>
#include <extlib/cereal/types/optional.hpp>

// #include "Player/Inventory.hpp

// typedef std::vector<std::optional<ItemCount>> ChestData;

// #include "World/ChestData.hpp"
#include "Player/InventoryData.hpp"
#include "Data/typedefs.hpp"

class ChestDataPool
{
public:
    ChestDataPool();

    // ID 0xFFFF returned means chest was not initialised, as data is full
    uint16_t createChest(const InventoryData& chestContents);
    uint16_t createChest(int capacity);

    void destroyChest(uint16_t id);

    InventoryData& getChestData(uint16_t id);

    InventoryData* getChestDataPtr(uint16_t id);

    template <class Archive>
    void serialize(Archive& ar, const std::uint32_t version)
    {
        ar(chestData, openDataSlots);
    }

    void mapVersions(const std::unordered_map<ItemType, ItemType> itemVersionMap)
    {
        for (auto& chestContents : chestData)
        {
            if (!chestContents.has_value())
            {
                continue;
            }

            chestContents->mapVersions(itemVersionMap);
        }   
    }

private:
    // 0xFFFF reserved for uninitialised chest / null
    std::vector<std::optional<InventoryData>> chestData;

    std::vector<uint16_t> openDataSlots;

    // uint16_t topDataSlot;
};

CEREAL_CLASS_VERSION(ChestDataPool, 1);