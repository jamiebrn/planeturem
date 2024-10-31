#pragma once

#include <vector>
#include <array>
#include <optional>
#include <unordered_map>
#include <fstream>

#include <Core/json.hpp>

#include <extlib/cereal/archives/binary.hpp>
#include <extlib/cereal/types/vector.hpp>
#include <extlib/cereal/types/utility.hpp>
#include <extlib/cereal/types/optional.hpp>

#include "Data/typedefs.hpp"
#include "Data/ItemDataLoader.hpp"
#include "Data/ObjectDataLoader.hpp"
#include "Data/ToolData.hpp"
#include "Data/ToolDataLoader.hpp"

// static constexpr unsigned int INVENTORY_STACK_SIZE = 20;
// static constexpr unsigned int MAX_INVENTORY_SIZE = 32;

class InventoryData
{
public:
    InventoryData() = default;
    InventoryData(int size);

    int addItem(ItemType item, int amount);

    void takeItem(ItemType item, int amount);

    void addItemAtIndex(int index, ItemType item, int amount);

    void takeItemAtIndex(int index, int amount);

    std::unordered_map<ItemType, unsigned int> getTotalItemCount() const;

    std::optional<ItemCount>& getItemSlotData(int index);

    bool isEmpty() const;

    int getProjectileCountForWeapon(ToolType weapon) const;

    inline int getSize() {return inventoryData.size();}

    inline const std::vector<std::optional<ItemCount>>& getData() const {return inventoryData;}
    inline std::vector<std::optional<ItemCount>>& getData() {return inventoryData;}

    // Save / load

    template <class Archive>
    void serialize(Archive& ar)
    {
        ar(inventoryData);
    }

    void mapVersions(const std::unordered_map<ItemType, ItemType>& itemVersionMap)
    {
        for (int i = 0; i < inventoryData.size(); i++)
        {
            if (!inventoryData[i].has_value())
            {
                continue;
            }

            inventoryData[i]->first = itemVersionMap.at(inventoryData[i]->first);
        }
    }

private:
    std::vector<std::optional<ItemCount>> inventoryData;

};

// Save / load
void to_json(nlohmann::json& json, const InventoryData& inventory);
void from_json(const nlohmann::json& json, InventoryData& inventory);